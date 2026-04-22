#include "scorer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int clamp_score(int value) {
    if (value < 0) {
        return 0;
    }
    if (value > 100) {
        return 100;
    }
    return value;
}

static int contains_case_insensitive(const char *text, const char *needle) {
    size_t nlen = strlen(needle);
    size_t tlen = strlen(text);
    size_t i = 0;
    size_t j = 0;

    if (nlen == 0 || tlen < nlen) {
        return 0;
    }

    for (i = 0; i <= tlen - nlen; ++i) {
        for (j = 0; j < nlen; ++j) {
            unsigned char a = (unsigned char)text[i + j];
            unsigned char b = (unsigned char)needle[j];
            if (tolower(a) != tolower(b)) {
                break;
            }
        }
        if (j == nlen) {
            return 1;
        }
    }

    return 0;
}

static int count_keyword_hits(const char *text, const char *const *keywords, size_t keyword_count) {
    size_t i = 0;
    int hits = 0;
    for (i = 0; i < keyword_count; ++i) {
        if (contains_case_insensitive(text, keywords[i])) {
            ++hits;
        }
    }
    return hits;
}

static int has_digit(const char *text) {
    while (*text != '\0') {
        if (isdigit((unsigned char)*text)) {
            return 1;
        }
        ++text;
    }
    return 0;
}

static size_t estimate_sentence_count(const char *text) {
    size_t count = 0;
    while (*text != '\0') {
        if (*text == '.' || *text == '?' || *text == '!') {
            ++count;
        }
        ++text;
    }
    return count;
}

const char *dimension_name(int dimension_id) {
    static const char *names[DIM_COUNT] = {
        "clarity",
        "context",
        "constraints",
        "output_format",
        "examples"
    };
    if (dimension_id < 0 || dimension_id >= DIM_COUNT) {
        return "unknown";
    }
    return names[dimension_id];
}

static int score_clarity(const char *prompt) {
    static const char *const verbs[] = {"write", "create", "generate", "explain", "summarize", "analyze", "design", "build"};
    static const char *const objective_terms[] = {"task", "goal", "please", "need", "want"};
    static const char *const ambiguous_terms[] = {"something", "stuff", "good", "nice", "better"};

    int score = 25;
    size_t len = strlen(prompt);
    int verb_hits = count_keyword_hits(prompt, verbs, sizeof(verbs) / sizeof(verbs[0]));
    int objective_hits = count_keyword_hits(prompt, objective_terms, sizeof(objective_terms) / sizeof(objective_terms[0]));
    int ambiguous_hits = count_keyword_hits(prompt, ambiguous_terms, sizeof(ambiguous_terms) / sizeof(ambiguous_terms[0]));

    if (len > 25) {
        score += 20;
    }
    if (len > 100) {
        score += 10;
    }
    score += verb_hits * 8;
    score += objective_hits * 6;
    if (estimate_sentence_count(prompt) > 1) {
        score += 10;
    }
    score -= ambiguous_hits * 12;
    return clamp_score(score);
}

static int score_context(const char *prompt) {
    static const char *const keywords[] = {
        "you are", "as a", "audience", "background", "context", "for ", "about ", "domain", "role"
    };
    int hits = count_keyword_hits(prompt, keywords, sizeof(keywords) / sizeof(keywords[0]));
    int score = 30 + hits * 20;
    return clamp_score(score);
}

static int score_constraints(const char *prompt) {
    static const char *const keywords[] = {
        "must", "should", "do not", "don't", "without", "limit", "at most", "exactly", "only", "no ", "under ", "at least"
    };
    int hits = count_keyword_hits(prompt, keywords, sizeof(keywords) / sizeof(keywords[0]));
    int score = hits * 25;
    if (has_digit(prompt)) {
        score += 25;
    }
    return clamp_score(score);
}

static int score_output_format(const char *prompt) {
    static const char *const keywords[] = {
        "json", "bullet", "table", "markdown", "list", "format", "section", "csv", "yaml", "xml"
    };
    int hits = count_keyword_hits(prompt, keywords, sizeof(keywords) / sizeof(keywords[0]));
    int score = 30 + hits * 20;
    return clamp_score(score);
}

static int score_examples(const char *prompt) {
    static const char *const keywords[] = {
        "example", "e.g.", "for instance", "sample", "input:", "output:"
    };
    int hits = count_keyword_hits(prompt, keywords, sizeof(keywords) / sizeof(keywords[0]));
    int score = hits * 35;
    return clamp_score(score);
}

static void push_feedback(PromptScore *score, const char *message) {
    if (score->feedback_count >= MAX_FEEDBACK_ITEMS) {
        return;
    }
    strncpy(score->feedback[score->feedback_count], message, MAX_FEEDBACK_LEN - 1);
    score->feedback[score->feedback_count][MAX_FEEDBACK_LEN - 1] = '\0';
    ++score->feedback_count;
}

static void generate_feedback(PromptScore *score) {
    if (score->dimension_scores[DIM_CLARITY] < 60) {
        push_feedback(score, "Improve clarity: use specific action verbs and remove vague terms.");
    }
    if (score->dimension_scores[DIM_CONTEXT] < 60) {
        push_feedback(score, "Add context: define role, audience, and domain/background.");
    }
    if (score->dimension_scores[DIM_CONSTRAINTS] < 60) {
        push_feedback(score, "Add constraints: include explicit limits, rules, or measurable requirements.");
    }
    if (score->dimension_scores[DIM_OUTPUT_FORMAT] < 60) {
        push_feedback(score, "Define output format: ask for a specific structure (e.g., JSON, bullets, table).");
    }
    if (score->dimension_scores[DIM_EXAMPLES] < 60) {
        push_feedback(score, "Provide examples: include sample input/output or reference examples.");
    }
    if (score->feedback_count == 0) {
        push_feedback(score, "Strong prompt. Keep the structure and constraints while refining details.");
    }
}

int evaluate_prompt(const char *prompt, PromptScore *out) {
    int sum = 0;
    int i = 0;

    if (prompt == NULL || out == NULL) {
        return -1;
    }
    if (prompt[0] == '\0') {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->dimension_scores[DIM_CLARITY] = score_clarity(prompt);
    out->dimension_scores[DIM_CONTEXT] = score_context(prompt);
    out->dimension_scores[DIM_CONSTRAINTS] = score_constraints(prompt);
    out->dimension_scores[DIM_OUTPUT_FORMAT] = score_output_format(prompt);
    out->dimension_scores[DIM_EXAMPLES] = score_examples(prompt);

    for (i = 0; i < DIM_COUNT; ++i) {
        sum += out->dimension_scores[i];
    }
    out->overall_score = clamp_score(sum / DIM_COUNT);
    generate_feedback(out);
    return 0;
}

void print_human_report(FILE *stream, const PromptScore *score) {
    size_t i = 0;

    fprintf(stream, "Overall score: %d/100\n", score->overall_score);
    fprintf(stream, "Dimension breakdown:\n");
    for (i = 0; i < DIM_COUNT; ++i) {
        fprintf(stream, "  - %-13s %d/100\n", dimension_name((int)i), score->dimension_scores[i]);
    }
    fprintf(stream, "Feedback:\n");
    for (i = 0; i < score->feedback_count; ++i) {
        fprintf(stream, "  - %s\n", score->feedback[i]);
    }
}

void print_json_report(FILE *stream, const PromptScore *score) {
    size_t i = 0;

    fprintf(stream, "{\n");
    fprintf(stream, "  \"overall\": %d,\n", score->overall_score);
    fprintf(stream, "  \"dimensions\": {\n");
    for (i = 0; i < DIM_COUNT; ++i) {
        fprintf(stream, "    \"%s\": %d%s\n",
                dimension_name((int)i),
                score->dimension_scores[i],
                (i + 1 < DIM_COUNT) ? "," : "");
    }
    fprintf(stream, "  },\n");
    fprintf(stream, "  \"feedback\": [\n");
    for (i = 0; i < score->feedback_count; ++i) {
        fprintf(stream, "    \"%s\"%s\n", score->feedback[i], (i + 1 < score->feedback_count) ? "," : "");
    }
    fprintf(stream, "  ]\n");
    fprintf(stream, "}\n");
}
