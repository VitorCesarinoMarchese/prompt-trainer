#ifndef TUI_TEXTWRAP_H
#define TUI_TEXTWRAP_H

#include <stddef.h>

typedef struct {
    int cursor_row;
    int cursor_col;
    int total_rows;
} TuiWrapCursorInfo;

int tui_textwrap_cursor_info(const char *text, size_t text_len, size_t cursor_index, int width, TuiWrapCursorInfo *out);
int tui_textwrap_get_row(const char *text, size_t text_len, int width, int target_row, char *out, size_t out_cap);

#endif
