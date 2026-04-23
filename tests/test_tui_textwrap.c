#include "tui/textwrap.h"

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
    TuiWrapCursorInfo info;
    char row[32];

    expect_true(tui_textwrap_cursor_info("hello", 5, 5, 10, &info) == 0, "cursor info should compute");
    expect_true(info.cursor_row == 0, "short text cursor row");
    expect_true(info.cursor_col == 5, "short text cursor col");
    expect_true(info.total_rows == 1, "short text row count");

    expect_true(tui_textwrap_cursor_info("abcdefghij", 10, 10, 5, &info) == 0, "wrap cursor info should compute");
    expect_true(info.cursor_row == 2, "wrapped cursor row should match width");
    expect_true(info.cursor_col == 0, "wrapped cursor col should reset");
    expect_true(info.total_rows == 3, "wrapped total rows");

    expect_true(tui_textwrap_cursor_info("ab\ncd", 5, 5, 10, &info) == 0, "newline cursor info should compute");
    expect_true(info.cursor_row == 1, "newline cursor row");
    expect_true(info.cursor_col == 2, "newline cursor col");
    expect_true(tui_textwrap_total_rows("ab\ncd", 5, 10) == 2, "total rows should handle newline");
    expect_true(tui_textwrap_total_rows("abcdefghij", 10, 5) == 3, "total rows should handle hard wraps");
    expect_true(tui_textwrap_total_rows("abcdef", 6, 3) == 3, "exact-width text should keep deterministic trailing row");

    expect_true(tui_textwrap_get_row("ab\ncdef", 7, 3, 1, row, sizeof(row)) == 1, "row extraction should work");
    expect_true(strcmp(row, "cde") == 0, "row extraction should honor width");
    expect_true(tui_textwrap_get_row("abcdef", 6, 3, 1, row, sizeof(row)) == 1, "exact-width middle row should render");
    expect_true(strcmp(row, "def") == 0, "exact-width middle row content should match");
    expect_true(tui_textwrap_get_row("abcdef", 6, 3, 2, row, sizeof(row)) == 1, "exact-width trailing row should exist");
    expect_true(strcmp(row, "") == 0, "exact-width trailing row should be empty");
    expect_true(tui_textwrap_get_row("ab", 2, 5, 3, row, sizeof(row)) == 0, "row extraction out-of-range should fail");

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }

    printf("All tui textwrap tests passed.\n");
    return 0;
}
