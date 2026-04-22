#include "tui/layout.h"

#include <stdio.h>

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void) {
    TuiLayout layout;
    tui_layout_compute(30, 100, 3, &layout);

    expect_true(layout.output_y == 0, "output starts at top");
    expect_true(layout.output_h > layout.input_h, "output should be larger than input");
    expect_true(layout.status_h == 1, "status line has height 1");
    expect_true(layout.status_y == 29, "status should be bottom line");
    expect_true(layout.compact_mode == 0, "normal terminal should not use compact mode");
    expect_true(layout.output_inner_w > 0, "output interior width should be positive");
    expect_true(layout.input_inner_w > 0, "input interior width should be positive");
    expect_true(layout.output_inner_y + layout.output_inner_h <= layout.input_y, "output interior should stay inside output panel");
    expect_true(layout.input_inner_y + layout.input_inner_h <= layout.status_y, "input interior should stay inside input panel");

    tui_layout_compute(8, 50, 10, &layout);
    expect_true(layout.output_h >= 1, "output keeps visible height");
    expect_true(layout.input_h >= 1, "input keeps visible height");
    expect_true(layout.cols == 50, "layout stores cols");
    expect_true(layout.compact_mode == 1, "small terminal should use compact mode");
    expect_true(layout.status_y + layout.status_h <= layout.rows, "status should stay inside screen bounds");
    expect_true(layout.output_inner_h >= 1, "output interior keeps at least one row");
    expect_true(layout.input_inner_h >= 1, "input interior keeps at least one row");
    expect_true(layout.output_inner_x >= 0 && layout.input_inner_x >= 0, "interior x should be non-negative");

    tui_layout_compute(4, 8, 2, &layout);
    expect_true(layout.rows == 4, "very small layout stores rows");
    expect_true(layout.cols == 8, "very small layout stores cols");
    expect_true(layout.status_y < layout.rows, "status y should stay in bounds on tiny terminal");
    expect_true(layout.output_h >= 1, "tiny terminal should keep output visible");
    expect_true(layout.input_h >= 1, "tiny terminal should keep input visible");
    expect_true(layout.output_inner_w >= 1, "tiny terminal output interior width should be non-zero");
    expect_true(layout.input_inner_w >= 1, "tiny terminal input interior width should be non-zero");

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All tui layout tests passed.\n");
    return 0;
}
