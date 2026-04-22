#ifndef TUI_HISTORY_H
#define TUI_HISTORY_H

#include <stddef.h>

#include "tui/state.h"

typedef struct {
    TuiMessage messages[TUI_MAX_MESSAGES];
    size_t count;
    int scroll;
} TuiHistory;

void tui_history_init(TuiHistory *history);
int tui_history_append(TuiHistory *history, TuiMessageType type, const char *text);
void tui_history_scroll_lines(TuiHistory *history, int delta, int max_scroll);
void tui_history_scroll_pages(TuiHistory *history, int page_delta, int page_size, int max_scroll);
int tui_history_total_rows(const TuiHistory *history, int width);
int tui_history_max_scroll_rows(const TuiHistory *history, int width, int viewport_rows);
int tui_history_render_row(const TuiHistory *history, int width, int row_index, char *out, size_t out_cap);

#endif
