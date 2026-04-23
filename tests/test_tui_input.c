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
    TuiWrapCursorInfo visual;

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

    tui_input_init(&state);
    type_text(&state, "abcdefghij");
    expect_true(tui_input_visual_info(&state, 5, &visual) == 0, "visual info should compute");
    expect_true(visual.cursor_row == 2, "visual row should track wrapped cursor");
    expect_true(visual.cursor_col == 0, "visual col should reset after hard wrap");
    expect_true(visual.total_rows == 3, "visual total rows should include wraps");
    expect_true(tui_input_wrapped_rows(&state, 5) == 3, "wrapped rows should match visual info");
    memset(submitted, 0, sizeof(submitted));
    expect_true(tui_input_visual_row(&state, 5, 1, submitted, sizeof(submitted)) == 1, "visual row extraction should work");
    expect_true(strcmp(submitted, "fghij") == 0, "visual row extraction should keep wrapped row content");
    expect_true(strlen(submitted) <= 5, "visual row extraction should not exceed render width");
    expect_true(tui_input_visual_row(&state, 5, 99, submitted, sizeof(submitted)) == 0, "out-of-range visual row should fail");
    tui_input_adjust_viewport(&state, 5, 2);
    expect_true(tui_input_view_top_row(&state) == 1, "viewport should follow wrapped cursor");
    tui_input_adjust_viewport(&state, 3, 2);
    expect_true(tui_input_view_top_row(&state) == 2, "viewport should recompute when width shrinks");
    tui_input_adjust_viewport(&state, 8, 2);
    expect_true(tui_input_view_top_row(&state) == 0, "viewport should clamp when width grows");

    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_handle_key(&state, TUI_KEY_LEFT, &outcome, submitted, sizeof(submitted));
    tui_input_adjust_viewport(&state, 5, 2);
    expect_true(tui_input_view_top_row(&state) == 0, "viewport should clamp when cursor moves up");

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
