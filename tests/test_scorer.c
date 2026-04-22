#include "scorer.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void test_high_quality_prompt_scores_well(void) {
    const char *prompt =
        "You are a senior C engineer writing for beginner developers. "
        "Create a markdown table with 5 debugging tips for segmentation faults. "
        "Use exactly 5 rows and keep each tip under 20 words. "
        "Example output: | tip | reason |";
    PromptScore score;
    int rc = evaluate_prompt(prompt, &score);

    expect_true(rc == 0, "high quality prompt should evaluate successfully");
    expect_true(score.overall_score >= 70, "high quality prompt should have overall >= 70");
    expect_true(score.dimension_scores[DIM_CONTEXT] >= 60, "high quality prompt should score context >= 60");
    expect_true(score.dimension_scores[DIM_CONSTRAINTS] >= 60, "high quality prompt should score constraints >= 60");
    expect_true(score.dimension_scores[DIM_OUTPUT_FORMAT] >= 60, "high quality prompt should score output format >= 60");
}

static void test_low_quality_prompt_gets_feedback(void) {
    const char *prompt = "Do something nice.";
    PromptScore score;
    int rc = evaluate_prompt(prompt, &score);

    expect_true(rc == 0, "low quality prompt should evaluate successfully");
    expect_true(score.overall_score <= 45, "low quality prompt should score low overall");
    expect_true(score.feedback_count >= 3, "low quality prompt should produce multiple feedback items");
}

static void test_empty_prompt_is_rejected(void) {
    PromptScore score;
    int rc = evaluate_prompt("", &score);
    expect_true(rc != 0, "empty prompt should be rejected");
}

int main(void) {
    test_high_quality_prompt_scores_well();
    test_low_quality_prompt_gets_feedback();
    test_empty_prompt_is_rejected();

    if (failures != 0) {
        fprintf(stderr, "Total failures: %d\n", failures);
        return 1;
    }
    printf("All scorer tests passed.\n");
    return 0;
}
