#ifndef SCORER_H
#define SCORER_H

#include <stddef.h>
#include <stdio.h>

enum {
    DIM_CLARITY = 0,
    DIM_CONTEXT = 1,
    DIM_CONSTRAINTS = 2,
    DIM_OUTPUT_FORMAT = 3,
    DIM_EXAMPLES = 4,
    DIM_COUNT = 5
};

#define MAX_FEEDBACK_ITEMS 8
#define MAX_FEEDBACK_LEN 160

typedef struct {
    int dimension_scores[DIM_COUNT];
    int overall_score;
    char feedback[MAX_FEEDBACK_ITEMS][MAX_FEEDBACK_LEN];
    size_t feedback_count;
} PromptScore;

int evaluate_prompt(const char *prompt, PromptScore *out);
void print_human_report(FILE *stream, const PromptScore *score);
void print_json_report(FILE *stream, const PromptScore *score);
const char *dimension_name(int dimension_id);

#endif
