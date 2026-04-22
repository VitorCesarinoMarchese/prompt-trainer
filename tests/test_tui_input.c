#include "tui/input.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void type_text(TuiInputState *state, const char *text) {
    while (*text != '\0') {
        TuiInputOutcome outcome;
        char submitted[TUI_MAX_INPUT];
        (void)tui_input_handle_key(state, (unsigned char)*text, &outcome, submitted, sizeof(submitted));
        ++text;
    }
}

int main(void) {
    TuiInputState state;
    TuiInputOutcome outcome;
    char submitted[TUI_MAX_INPUT];

    tui_input_init(&state);
    type_text(&state, "hello");
    expect_true(state.len == 5, "typed text should be present");

    memset(&outcome, 0, sizeof(outcome));
    memset(submitted, 0, sizeof(submitted));
    expect_true(tui_input_handle_key(&state, '\n', &outcome, submitted, sizeof(submitted)) == 0, "enter should be handled");
    expect_true(outcome.action == TUI_INPUT_ACTION_SUBMIT, "enter should submit");
    expect_true(strcmp(submitted, "hello") == 0, "submitted text should match input");
    expect_true(state.len == 0, "input should clear after submit");

    type_text(&state, "line1");
    tui_input_handle_key(&state, TUI_KEY_SHIFT_ENTER, &outcome, submitted, sizeof(submitted));
    type_text(&state, "line2");
    expect_true(tui_input_line_count(&state) == 2, "shift+enter should add newline");
    memset(&outcome, 0, sizeof(outcome));
    memset(submitted, 0, sizeof(submitted));
    expect_true(tui_input_handle_key(&state, TUI_KEY_ENTER, &outcome, submitted, sizeof(submitted)) == 0, "KEY_ENTER should be handled");
    expect_true(outcome.action == TUI_INPUT_ACTION_SUBMIT, "KEY_ENTER should submit");
    expect_true(state.len == 0, "input should clear after KEY_ENTER submit");

    memset(&outcome, 0, sizeof(outcome));
    state.len = 0;
    state.cursor = 0;
    state.buffer[0] = '\0';
    tui_input_handle_key(&state, TUI_KEY_UP, &outcome, submitted, sizeof(submitted));
    expect_true(strcmp(state.buffer, "line1\nline2") == 0, "up on empty input should recall latest input history");

    memset(&outcome, 0, sizeof(outcome));
    tui_input_handle_key(&state, TUI_KEY_PGUP, &outcome, submitted, sizeof(submitted));
    expect_true(outcome.scroll_page_delta == 1, "page up should request upward page scroll");

    memset(&outcome, 0, sizeof(outcome));
    tui_input_handle_key(&state, 3, &outcome, submitted, sizeof(submitted));
    expect_true(outcome.action == TUI_INPUT_ACTION_QUIT, "ctrl+c should quit");

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All tui input tests passed.\n");
    return 0;
}
