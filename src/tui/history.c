#include "tui/history.h"

#include <stdio.h>
#include <string.h>

static const char *message_prefix(TuiMessageType type) {
    switch (type) {
        case TUI_MSG_USER:
            return "You> ";
        case TUI_MSG_RESULT:
            return "App> ";
        case TUI_MSG_STATUS:
            return "Info> ";
        case TUI_MSG_ERROR:
            return "Err> ";
        default:
            return "> ";
    }
}

static int safe_width(int width) {
    if (width < 1) {
        return 1;
    }
    return width;
}

static int line_rows_for_text(const char *text, int width) {
    int rows = 1;
    int col = 0;
    int w = safe_width(width);
    while (*text != '\0') {
        if (*text == '\n') {
            rows++;
            col = 0;
        } else {
            col++;
            if (col >= w) {
                rows++;
                col = 0;
            }
        }
        ++text;
    }
    return rows;
}

static int message_rows(const TuiMessage *message, int width) {
    char full[TUI_MAX_MESSAGE_TEXT + 32];
    snprintf(full, sizeof(full), "%s%s", message_prefix(message->type), message->text);
    return line_rows_for_text(full, width);
}

static int extract_wrapped_row(const char *text, int width, int target_row, char *out, size_t out_cap) {
    int current_row = 0;
    int col = 0;
    size_t out_len = 0;
    int w = safe_width(width);
    const char *p = text;
    if (out_cap == 0) {
        return 0;
    }
    out[0] = '\0';

    while (1) {
        char ch = *p;
        int row_break = 0;
        if (ch == '\0') {
            row_break = 1;
        } else if (ch == '\n') {
            row_break = 1;
        } else {
            if (current_row == target_row && out_len + 1 < out_cap) {
                out[out_len++] = ch;
            }
            col++;
            if (col >= w) {
                row_break = 1;
            }
        }

        if (row_break) {
            if (current_row == target_row) {
                out[out_len] = '\0';
                return 1;
            }
            current_row++;
            col = 0;
            if (ch == '\0') {
                break;
            }
        }

        if (ch == '\0') {
            break;
        }
        p++;
    }

    return 0;
}

void tui_history_init(TuiHistory *history) {
    memset(history, 0, sizeof(*history));
}

int tui_history_append(TuiHistory *history, TuiMessageType type, const char *text) {
    size_t idx;
    if (history->count < TUI_MAX_MESSAGES) {
        idx = history->count++;
    } else {
        memmove(&history->messages[0], &history->messages[1], sizeof(TuiMessage) * (TUI_MAX_MESSAGES - 1));
        idx = TUI_MAX_MESSAGES - 1;
    }

    history->messages[idx].type = type;
    strncpy(history->messages[idx].text, text, TUI_MAX_MESSAGE_TEXT - 1);
    history->messages[idx].text[TUI_MAX_MESSAGE_TEXT - 1] = '\0';
    return 0;
}

void tui_history_scroll_lines(TuiHistory *history, int delta, int max_scroll) {
    int next = history->scroll + delta;
    if (next < 0) {
        next = 0;
    }
    if (next > max_scroll) {
        next = max_scroll;
    }
    history->scroll = next;
}

void tui_history_scroll_pages(TuiHistory *history, int page_delta, int page_size, int max_scroll) {
    tui_history_scroll_lines(history, page_delta * page_size, max_scroll);
}

int tui_history_total_rows(const TuiHistory *history, int width) {
    int rows = 0;
    size_t i;
    for (i = 0; i < history->count; ++i) {
        rows += message_rows(&history->messages[i], width);
    }
    return rows;
}

int tui_history_max_scroll_rows(const TuiHistory *history, int width, int viewport_rows) {
    int total = tui_history_total_rows(history, width);
    int visible = viewport_rows > 1 ? viewport_rows : 1;
    int max_scroll = total - visible;
    if (max_scroll < 0) {
        return 0;
    }
    return max_scroll;
}

int tui_history_render_row(const TuiHistory *history, int width, int row_index, char *out, size_t out_cap) {
    int row_cursor = 0;
    size_t i;
    if (row_index < 0) {
        return 0;
    }
    for (i = 0; i < history->count; ++i) {
        int rows = message_rows(&history->messages[i], width);
        if (row_index < row_cursor + rows) {
            char full[TUI_MAX_MESSAGE_TEXT + 32];
            int relative = row_index - row_cursor;
            snprintf(full, sizeof(full), "%s%s", message_prefix(history->messages[i].type), history->messages[i].text);
            return extract_wrapped_row(full, width, relative, out, out_cap);
        }
        row_cursor += rows;
    }
    return 0;
}
