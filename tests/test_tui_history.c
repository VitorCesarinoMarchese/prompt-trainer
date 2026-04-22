#include "tui/history.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void) {
    TuiHistory history;
    char line[TUI_MAX_MESSAGE_TEXT];
    tui_history_init(&history);

    expect_true(history.count == 0, "history should start empty");
    expect_true(tui_history_append(&history, TUI_MSG_USER, "hello") == 0, "append should work");
    expect_true(history.count == 1, "count should increase");
    expect_true(strcmp(history.messages[0].text, "hello") == 0, "message text should match");

    tui_history_scroll_lines(&history, 5, 20);
    expect_true(history.scroll == 5, "scroll lines should increase");

    tui_history_scroll_lines(&history, -100, 20);
    expect_true(history.scroll == 0, "scroll should clamp to zero");

    tui_history_scroll_pages(&history, 2, 4, 20);
    expect_true(history.scroll == 8, "page scrolling should multiply by page size");

    tui_history_scroll_pages(&history, 20, 4, 9);
    expect_true(history.scroll == 9, "scroll should clamp to max");

    tui_history_init(&history);
    tui_history_append(&history, TUI_MSG_USER, "1234567890");
    tui_history_append(&history, TUI_MSG_RESULT, "line1\nline2");
    expect_true(tui_history_total_rows(&history, 8) > 3, "wrapping should expand visual row count");
    expect_true(tui_history_max_scroll_rows(&history, 8, 2) >= 1, "max scroll should reflect wrapped rows");
    expect_true(tui_history_render_row(&history, 8, 0, line, sizeof(line)) == 1, "first row render should work");
    expect_true(strlen(line) > 0, "rendered row should not be empty");
    expect_true(strchr(line, '\n') == NULL, "rendered row should not contain raw newline");
    expect_true(tui_history_render_row(&history, 8, 999, line, sizeof(line)) == 0, "out-of-range row render should fail");

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All tui history tests passed.\n");
    return 0;
}
