#include "tui/layout.h"

static int clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void compute_panel_inner(int panel_y, int panel_h, int cols, int *inner_x, int *inner_y, int *inner_w, int *inner_h) {
    if (panel_h >= 3 && cols >= 4) {
        *inner_x = 1;
        *inner_y = panel_y + 1;
        *inner_w = cols - 2;
        *inner_h = panel_h - 2;
        return;
    }
    *inner_x = 0;
    *inner_y = panel_y;
    *inner_w = cols > 0 ? cols : 1;
    *inner_h = panel_h > 0 ? panel_h : 1;
}

void tui_layout_compute(int rows, int cols, int input_lines, TuiLayout *out) {
    int status_h;
    int available;
    int desired_input;
    int input_h;
    int output_h;

    if (rows < 1) {
        rows = 1;
    }
    if (cols < 1) {
        cols = 1;
    }

    status_h = rows >= 1 ? 1 : 0;
    available = rows - status_h;
    out->compact_mode = 0;

    if (available <= 0) {
        output_h = 0;
        input_h = 0;
        out->compact_mode = 1;
    } else if (available == 1) {
        output_h = 1;
        input_h = 0;
        out->compact_mode = 1;
    } else {
        desired_input = clamp_int(input_lines + 2, 3, 10);
        input_h = desired_input;
        if (input_h > available - 1) {
            input_h = available - 1;
        }
        if (input_h < 1) {
            input_h = 1;
        }
        output_h = available - input_h;
        if (output_h < 3 || input_h < 3 || cols < 40 || rows < 10) {
            out->compact_mode = 1;
        }
    }

    out->rows = rows;
    out->cols = cols;
    out->output_y = 0;
    out->output_h = output_h;
    compute_panel_inner(out->output_y, out->output_h, cols, &out->output_inner_x, &out->output_inner_y, &out->output_inner_w, &out->output_inner_h);
    out->input_y = output_h;
    out->input_h = input_h;
    compute_panel_inner(out->input_y, out->input_h, cols, &out->input_inner_x, &out->input_inner_y, &out->input_inner_w, &out->input_inner_h);
    out->status_y = output_h + input_h;
    out->status_h = status_h;
}
