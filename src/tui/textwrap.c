#include "tui/textwrap.h"

static int normalized_width(int width) {
    if (width < 1) {
        return 1;
    }
    return width;
}

int tui_textwrap_cursor_info(const char *text, size_t text_len, size_t cursor_index, int width, TuiWrapCursorInfo *out) {
    int row = 0;
    int col = 0;
    int w = normalized_width(width);
    size_t i;

    if (text == 0 || out == 0) {
        return -1;
    }
    if (cursor_index > text_len) {
        cursor_index = text_len;
    }

    out->cursor_row = 0;
    out->cursor_col = 0;
    out->total_rows = 1;

    for (i = 0; i < text_len; ++i) {
        if (i == cursor_index) {
            out->cursor_row = row;
            out->cursor_col = col;
        }

        if (text[i] == '\n') {
            row++;
            col = 0;
            continue;
        }

        col++;
        if (col >= w) {
            row++;
            col = 0;
        }
    }

    if (cursor_index == text_len) {
        out->cursor_row = row;
        out->cursor_col = col;
    }
    out->total_rows = row + 1;
    return 0;
}

int tui_textwrap_get_row(const char *text, size_t text_len, int width, int target_row, char *out, size_t out_cap) {
    int current_row = 0;
    int col = 0;
    int w = normalized_width(width);
    size_t i;
    size_t out_len = 0;

    if (text == 0 || out == 0 || out_cap == 0 || target_row < 0) {
        return 0;
    }
    out[0] = '\0';

    for (i = 0; i < text_len; ++i) {
        char ch = text[i];

        if (ch == '\n') {
            if (current_row == target_row) {
                out[out_len] = '\0';
                return 1;
            }
            current_row++;
            col = 0;
            continue;
        }

        if (current_row == target_row && out_len + 1 < out_cap) {
            out[out_len++] = ch;
        }

        col++;
        if (col >= w) {
            if (current_row == target_row) {
                out[out_len] = '\0';
                return 1;
            }
            current_row++;
            col = 0;
        }
    }

    if (current_row == target_row) {
        out[out_len] = '\0';
        return 1;
    }
    return 0;
}
