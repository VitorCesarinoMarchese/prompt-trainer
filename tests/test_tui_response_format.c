#include "tui/response_format.h"

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
    PromptScore low;
    PromptScore high;
    char out[2048];

    memset(&low, 0, sizeof(low));
    low.overall_score = 58;
    low.dimension_scores[DIM_CLARITY] = 25;
    low.dimension_scores[DIM_CONTEXT] = 30;
    low.dimension_scores[DIM_CONSTRAINTS] = 0;
    low.dimension_scores[DIM_OUTPUT_FORMAT] = 30;
    low.dimension_scores[DIM_EXAMPLES] = 0;

    expect_true(tui_format_score_response(&low, out, sizeof(out)) == 0, "format low score should work");
    expect_true(strstr(out, "Score 58/100") != NULL, "response should include score header");
    expect_true(strstr(out, "- Clarity: 25") != NULL, "response should include clarity section line");
    expect_true(strstr(out, "- Format: 30") != NULL, "response should include format line");
    expect_true(strstr(out, "Improvements:\n1.") != NULL, "response should include numbered improvements");
    expect_true(strstr(out, "2.") != NULL, "response should include multiple improvements");

    memset(&high, 0, sizeof(high));
    high.overall_score = 95;
    high.dimension_scores[DIM_CLARITY] = 95;
    high.dimension_scores[DIM_CONTEXT] = 95;
    high.dimension_scores[DIM_CONSTRAINTS] = 95;
    high.dimension_scores[DIM_OUTPUT_FORMAT] = 95;
    high.dimension_scores[DIM_EXAMPLES] = 95;

    expect_true(tui_format_score_response(&high, out, sizeof(out)) == 0, "format high score should work");
    expect_true(strstr(out, "Improvements:\nNone.") != NULL, "high score should report no improvements");
    expect_true(strstr(out, "1.") == NULL, "high score should not include numbered improvements");

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All tui response format tests passed.\n");
    return 0;
}
