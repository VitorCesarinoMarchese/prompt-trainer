#include "tui/render.h"

#include <curses.h>
#include <string.h>

static void draw_output_panel(const TuiLayout *layout,
                              const TuiHistory *history) {
  int inner_h = tui_layout_output_view_rows(layout);
  int width = tui_layout_output_view_width(layout);
  int total_rows;
  int start;
  int i;
  char line[TUI_MAX_MESSAGE_TEXT + 32];

  mvhline(layout->output_y, 0, ' ', layout->cols);
  for (i = 1; i < layout->output_h - 1; ++i) {
    mvhline(layout->output_y + i, 0, ' ', layout->cols);
  }
  if (layout->output_h > 0) {
    mvhline(layout->output_y + layout->output_h - 1, 0, ' ', layout->cols);
  }
  if (layout->output_h > 0) {
    mvprintw(layout->output_y, 0, "+ Output");
  }

  if (inner_h <= 0 || width <= 0) {
    return;
  }

  total_rows = tui_history_total_rows(history, width);
  start = total_rows - inner_h - history->scroll;
  if (start < 0) {
    start = 0;
  }

  for (i = 0; i < inner_h; ++i) {
    int row_index = start + i;
    if (row_index >= total_rows) {
      break;
    }
    if (tui_history_render_row(history, width, row_index, line, sizeof(line)) !=
        1) {
      continue;
    }
    mvaddnstr(layout->output_inner_y + i, layout->output_inner_x, line, width);
  }
}

static void draw_input_panel(const TuiLayout *layout,
                             const TuiInputState *input) {
  int i;
  int inner_h = tui_layout_input_view_rows(layout);
  int inner_w = tui_layout_input_view_width(layout);
  int cursor_row = 0;
  int cursor_col = 0;
  TuiWrapCursorInfo visual;
  memset(&visual, 0, sizeof(visual));
  int start_row = 0;
  int total_lines;
  int visible_lines = inner_h > 0 ? inner_h : 1;
  char line[TUI_MAX_INPUT];

  for (i = 0; i < layout->input_h; ++i) {
    mvhline(layout->input_y + i, 0, ' ', layout->cols);
  }
  if (layout->input_h > 0) {
    mvprintw(layout->input_y, 0, "+ Input");
  }

  if (inner_h <= 0 || inner_w <= 0) {
    return;
  }

  if (tui_input_visual_info(input, inner_w, &visual) != 0) {
    visual.cursor_row = 0;
    visual.cursor_col = 0;
    visual.total_rows = 1;
  }
  total_lines = visual.total_rows;
  start_row = tui_input_view_top_row(input);
  if (start_row < 0) {
    start_row = 0;
  }
  if (start_row > total_lines - visible_lines) {
    start_row = total_lines - visible_lines;
  }
  if (start_row < 0) {
    start_row = 0;
  }

  for (i = 0; i < visible_lines; ++i) {
    int row_index = start_row + i;
    if (row_index >= total_lines) {
      break;
    }
    if (tui_input_visual_row(input, inner_w, row_index, line, sizeof(line)) !=
        1) {
      continue;
    }
    mvaddnstr(layout->input_inner_y + i, layout->input_inner_x, line, inner_w);
  }

  cursor_row = visual.cursor_row;
  cursor_col = visual.cursor_col;

  if (cursor_row < start_row) {
    cursor_row = start_row;
  }
  if (cursor_col < 0) {
    cursor_col = 0;
  }
  if (cursor_col >= inner_w) {
    cursor_col = inner_w - 1;
  }
  if (cursor_row >= start_row + visible_lines) {
    cursor_row = start_row + visible_lines - 1;
  }

  move(layout->input_inner_y + (cursor_row - start_row),
       layout->input_inner_x + cursor_col);
}

static void draw_input_cursor(const TuiLayout *layout,
                              const TuiInputState *input) {
  int inner_w = tui_layout_input_view_width(layout);
  int inner_h = tui_layout_input_view_rows(layout);
  TuiWrapCursorInfo visual;
  memset(&visual, 0, sizeof(visual));
  if (tui_input_visual_info(input, inner_w, &visual) != 0) {
    visual.cursor_row = 0;
    visual.cursor_col = 0;
  }
  int start_row = tui_input_view_top_row(input);
  if (start_row < 0)
    start_row = 0;
  int visible_lines = inner_h > 0 ? inner_h : 1;
  int cursor_row = visual.cursor_row;
  int cursor_col = visual.cursor_col;
  if (cursor_row < start_row)
    cursor_row = start_row;
  if (cursor_col < 0)
    cursor_col = 0;
  if (cursor_col >= inner_w)
    cursor_col = inner_w - 1;
  if (cursor_row >= start_row + visible_lines)
    cursor_row = start_row + visible_lines - 1;
  move(layout->input_inner_y + (cursor_row - start_row),
       layout->input_inner_x + cursor_col);
}

void tui_render_screen(const TuiLayout *layout, const TuiHistory *history,
                       const TuiInputState *input, const char *status_text) {
  erase();
  draw_output_panel(layout, history);
  draw_input_panel(layout, input);
  if (layout->status_h > 0) {
    mvprintw(layout->status_y, 0, "%-*.*s", layout->cols, layout->cols,
             status_text);
  }
  draw_input_cursor(layout, input);
  refresh();
}
