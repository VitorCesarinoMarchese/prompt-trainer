#include "tui/response_format.h"

#include <stdarg.h>
#include <stdio.h>

static int appendf(char *out, size_t out_cap, size_t *offset, const char *fmt, ...) {
    int written;
    va_list args;

    if (*offset >= out_cap) {
        return -1;
    }

    va_start(args, fmt);
    written = vsnprintf(out + *offset, out_cap - *offset, fmt, args);
    va_end(args);

    if (written < 0) {
        return -1;
    }
    if ((size_t)written >= out_cap - *offset) {
        *offset = out_cap;
        return -1;
    }
    *offset += (size_t)written;
    return 0;
}

static int append_improvement(char *out, size_t out_cap, size_t *offset, int *idx, const char *text) {
    *idx += 1;
    return appendf(out, out_cap, offset, "%d. %s\n", *idx, text);
}

int tui_format_score_response(const PromptScore *score, char *out, size_t out_cap) {
    size_t offset = 0;
    int imp_idx = 0;

    if (score == NULL || out == NULL || out_cap == 0) {
        return -1;
    }
    out[0] = '\0';

    if (appendf(out, out_cap, &offset, "Score %d/100\n\n", score->overall_score) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "- Clarity: %d\n", score->dimension_scores[DIM_CLARITY]) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "- Context: %d\n", score->dimension_scores[DIM_CONTEXT]) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "- Constraints: %d\n", score->dimension_scores[DIM_CONSTRAINTS]) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "- Format: %d\n", score->dimension_scores[DIM_OUTPUT_FORMAT]) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "- Examples: %d\n\n", score->dimension_scores[DIM_EXAMPLES]) != 0) {
        return -1;
    }
    if (appendf(out, out_cap, &offset, "Improvements:\n") != 0) {
        return -1;
    }

    if (score->dimension_scores[DIM_CLARITY] < 60 &&
        append_improvement(out, out_cap, &offset, &imp_idx, "Improve clarity: use specific action verbs and remove vague terms.") != 0) {
        return -1;
    }
    if (score->dimension_scores[DIM_CONTEXT] < 60 &&
        append_improvement(out, out_cap, &offset, &imp_idx, "Add context: define role, audience, and domain/background.") != 0) {
        return -1;
    }
    if (score->dimension_scores[DIM_CONSTRAINTS] < 60 &&
        append_improvement(out, out_cap, &offset, &imp_idx, "Add constraints: include explicit limits, rules, or measurable requirements.") != 0) {
        return -1;
    }
    if (score->dimension_scores[DIM_OUTPUT_FORMAT] < 60 &&
        append_improvement(out, out_cap, &offset, &imp_idx, "Define output format: request a specific structure (JSON, bullets, table).") != 0) {
        return -1;
    }
    if (score->dimension_scores[DIM_EXAMPLES] < 60 &&
        append_improvement(out, out_cap, &offset, &imp_idx, "Provide examples: include sample input/output or reference examples.") != 0) {
        return -1;
    }

    if (imp_idx == 0) {
        if (appendf(out, out_cap, &offset, "None.\n") != 0) {
            return -1;
        }
    }

    return 0;
}
