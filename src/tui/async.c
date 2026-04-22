#include "tui/async.h"

#include <string.h>

static void fill_error(TuiAsyncResult *result, const char *prompt, const char *message) {
    result->ok = 0;
    strncpy(result->prompt, prompt, TUI_MAX_INPUT - 1);
    result->prompt[TUI_MAX_INPUT - 1] = '\0';
    strncpy(result->error, message, sizeof(result->error) - 1);
    result->error[sizeof(result->error) - 1] = '\0';
}

static void *tui_async_worker(void *arg) {
    TuiAsync *state = (TuiAsync *)arg;

    for (;;) {
        char prompt[TUI_MAX_INPUT];
        PromptScore score;
        int rc;

        pthread_mutex_lock(&state->mutex);
        while (state->running && state->job_count == 0) {
            pthread_cond_wait(&state->cv, &state->mutex);
        }
        if (!state->running) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }
        strncpy(prompt, state->job_queue[state->job_head], sizeof(prompt) - 1);
        prompt[sizeof(prompt) - 1] = '\0';
        state->job_head = (state->job_head + 1) % TUI_ASYNC_QUEUE_CAP;
        state->job_count--;
        state->worker_active = 1;
        pthread_mutex_unlock(&state->mutex);

        memset(&score, 0, sizeof(score));
        rc = evaluate_prompt(prompt, &score);

        pthread_mutex_lock(&state->mutex);
        state->worker_active = 0;
        if (state->result_count == TUI_ASYNC_QUEUE_CAP) {
            state->result_head = (state->result_head + 1) % TUI_ASYNC_QUEUE_CAP;
            state->result_count--;
        }
        memset(&state->result_queue[state->result_tail], 0, sizeof(state->result_queue[state->result_tail]));
        if (rc == 0) {
            state->result_queue[state->result_tail].ok = 1;
            strncpy(state->result_queue[state->result_tail].prompt, prompt, TUI_MAX_INPUT - 1);
            state->result_queue[state->result_tail].prompt[TUI_MAX_INPUT - 1] = '\0';
            state->result_queue[state->result_tail].score = score;
        } else {
            fill_error(&state->result_queue[state->result_tail], prompt, "Prompt is empty or invalid.");
        }
        state->result_tail = (state->result_tail + 1) % TUI_ASYNC_QUEUE_CAP;
        state->result_count++;
        pthread_mutex_unlock(&state->mutex);
    }

    return NULL;
}

int tui_async_init(TuiAsync *async_state) {
    memset(async_state, 0, sizeof(*async_state));
    if (pthread_mutex_init(&async_state->mutex, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&async_state->cv, NULL) != 0) {
        pthread_mutex_destroy(&async_state->mutex);
        return -1;
    }
    async_state->running = 1;
    if (pthread_create(&async_state->thread, NULL, tui_async_worker, async_state) != 0) {
        pthread_cond_destroy(&async_state->cv);
        pthread_mutex_destroy(&async_state->mutex);
        return -1;
    }
    return 0;
}

void tui_async_shutdown(TuiAsync *async_state) {
    pthread_mutex_lock(&async_state->mutex);
    async_state->running = 0;
    pthread_cond_signal(&async_state->cv);
    pthread_mutex_unlock(&async_state->mutex);
    pthread_join(async_state->thread, NULL);
    pthread_cond_destroy(&async_state->cv);
    pthread_mutex_destroy(&async_state->mutex);
}

int tui_async_submit(TuiAsync *async_state, const char *prompt) {
    int rc = 0;
    pthread_mutex_lock(&async_state->mutex);
    if (async_state->job_count == TUI_ASYNC_QUEUE_CAP) {
        rc = -1;
    } else {
        strncpy(async_state->job_queue[async_state->job_tail], prompt, TUI_MAX_INPUT - 1);
        async_state->job_queue[async_state->job_tail][TUI_MAX_INPUT - 1] = '\0';
        async_state->job_tail = (async_state->job_tail + 1) % TUI_ASYNC_QUEUE_CAP;
        async_state->job_count++;
        pthread_cond_signal(&async_state->cv);
    }
    pthread_mutex_unlock(&async_state->mutex);
    return rc;
}

int tui_async_poll(TuiAsync *async_state, TuiAsyncResult *out) {
    int ready = 0;
    pthread_mutex_lock(&async_state->mutex);
    if (async_state->result_count > 0) {
        *out = async_state->result_queue[async_state->result_head];
        async_state->result_head = (async_state->result_head + 1) % TUI_ASYNC_QUEUE_CAP;
        async_state->result_count--;
        ready = 1;
    }
    pthread_mutex_unlock(&async_state->mutex);
    return ready;
}

int tui_async_is_busy(TuiAsync *async_state) {
    int busy;
    pthread_mutex_lock(&async_state->mutex);
    busy = async_state->worker_active || async_state->job_count > 0 || async_state->result_count > 0;
    pthread_mutex_unlock(&async_state->mutex);
    return busy;
}

int tui_async_pending_count(TuiAsync *async_state) {
    int pending;
    pthread_mutex_lock(&async_state->mutex);
    pending = async_state->job_count + async_state->result_count + (async_state->worker_active ? 1 : 0);
    pthread_mutex_unlock(&async_state->mutex);
    return pending;
}
