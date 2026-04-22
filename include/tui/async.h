#ifndef TUI_ASYNC_H
#define TUI_ASYNC_H

#include <pthread.h>

#include "scorer.h"
#include "tui/state.h"

#define TUI_ASYNC_QUEUE_CAP 32

typedef struct {
    int ok;
    char prompt[TUI_MAX_INPUT];
    PromptScore score;
    char error[128];
} TuiAsyncResult;

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    int running;
    int worker_active;
    char job_queue[TUI_ASYNC_QUEUE_CAP][TUI_MAX_INPUT];
    int job_head;
    int job_tail;
    int job_count;
    TuiAsyncResult result_queue[TUI_ASYNC_QUEUE_CAP];
    int result_head;
    int result_tail;
    int result_count;
} TuiAsync;

int tui_async_init(TuiAsync *async_state);
void tui_async_shutdown(TuiAsync *async_state);
int tui_async_submit(TuiAsync *async_state, const char *prompt);
int tui_async_poll(TuiAsync *async_state, TuiAsyncResult *out);
int tui_async_is_busy(TuiAsync *async_state);
int tui_async_pending_count(TuiAsync *async_state);

#endif
