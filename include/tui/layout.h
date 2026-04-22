#ifndef TUI_LAYOUT_H
#define TUI_LAYOUT_H

typedef struct {
    int rows;
    int compact_mode;
    int output_y;
    int output_h;
    int output_inner_x;
    int output_inner_y;
    int output_inner_w;
    int output_inner_h;
    int input_y;
    int input_h;
    int input_inner_x;
    int input_inner_y;
    int input_inner_w;
    int input_inner_h;
    int status_y;
    int status_h;
    int cols;
} TuiLayout;

void tui_layout_compute(int rows, int cols, int input_lines, TuiLayout *out);
int tui_layout_output_view_rows(const TuiLayout *layout);
int tui_layout_output_view_width(const TuiLayout *layout);
int tui_layout_input_view_rows(const TuiLayout *layout);
int tui_layout_input_view_width(const TuiLayout *layout);

#endif
