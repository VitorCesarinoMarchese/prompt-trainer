#include "scorer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_usage(const char *prog_name) {
    fprintf(stderr,
            "Usage: %s [--json] [--interactive] [-p \"prompt\"] [prompt words...]\n"
            "Modes:\n"
            "  -p, --prompt TEXT   Evaluate prompt from argument\n"
            "  -i, --interactive   Read prompt from terminal/stdin\n"
            "      --json          Output machine-readable JSON\n"
            "  -h, --help          Show this help\n",
            prog_name);
}

static int append_chunk(char **buffer, size_t *len, size_t *cap, const char *chunk, size_t chunk_len) {
    size_t required = *len + chunk_len + 1;
    char *new_buffer = NULL;

    if (required > *cap) {
        size_t new_cap = (*cap == 0) ? 128 : *cap;
        while (new_cap < required) {
            new_cap *= 2;
        }
        new_buffer = realloc(*buffer, new_cap);
        if (new_buffer == NULL) {
            return -1;
        }
        *buffer = new_buffer;
        *cap = new_cap;
    }
    memcpy(*buffer + *len, chunk, chunk_len);
    *len += chunk_len;
    (*buffer)[*len] = '\0';
    return 0;
}

static char *dup_string(const char *text) {
    size_t len = strlen(text);
    char *copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, len + 1);
    return copy;
}

static char *read_all_from_stream(FILE *stream) {
    char *buffer = NULL;
    size_t len = 0;
    size_t cap = 0;
    char chunk[256];

    while (fgets(chunk, sizeof(chunk), stream) != NULL) {
        size_t chunk_len = strlen(chunk);
        if (append_chunk(&buffer, &len, &cap, chunk, chunk_len) != 0) {
            free(buffer);
            return NULL;
        }
    }
    if (buffer == NULL) {
        buffer = malloc(1);
        if (buffer == NULL) {
            return NULL;
        }
        buffer[0] = '\0';
    }
    return buffer;
}

static char *join_args(int start, int argc, char **argv) {
    char *buffer = NULL;
    size_t len = 0;
    size_t cap = 0;
    int i = 0;

    for (i = start; i < argc; ++i) {
        size_t arg_len = strlen(argv[i]);
        if (append_chunk(&buffer, &len, &cap, argv[i], arg_len) != 0) {
            free(buffer);
            return NULL;
        }
        if (i + 1 < argc && append_chunk(&buffer, &len, &cap, " ", 1) != 0) {
            free(buffer);
            return NULL;
        }
    }
    if (buffer == NULL) {
        buffer = malloc(1);
        if (buffer == NULL) {
            return NULL;
        }
        buffer[0] = '\0';
    }
    return buffer;
}

int main(int argc, char **argv) {
    PromptScore score;
    int as_json = 0;
    int interactive = 0;
    char *prompt = NULL;
    int i = 0;
    int positional_start = -1;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--json") == 0) {
            as_json = 1;
        } else if (strcmp(argv[i], "--interactive") == 0 || strcmp(argv[i], "-i") == 0) {
            interactive = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if ((strcmp(argv[i], "--prompt") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
            prompt = dup_string(argv[i + 1]);
            if (prompt == NULL) {
                fprintf(stderr, "Error: failed to allocate memory for prompt.\n");
                return 1;
            }
            ++i;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: unknown option '%s'.\n", argv[i]);
            print_usage(argv[0]);
            free(prompt);
            return 1;
        } else {
            positional_start = i;
            break;
        }
    }

    if (prompt == NULL && positional_start != -1) {
        prompt = join_args(positional_start, argc, argv);
    }

    if (prompt == NULL) {
        if (interactive || isatty(STDIN_FILENO)) {
            fprintf(stderr, "Enter your prompt and press Ctrl-D when done:\n");
        }
        prompt = read_all_from_stream(stdin);
    }

    if (prompt == NULL) {
        fprintf(stderr, "Error: failed to read prompt.\n");
        return 1;
    }
    if (evaluate_prompt(prompt, &score) != 0) {
        fprintf(stderr, "Error: prompt is empty or invalid.\n");
        free(prompt);
        return 1;
    }

    if (as_json) {
        print_json_report(stdout, &score);
    } else {
        print_human_report(stdout, &score);
    }

    free(prompt);
    return 0;
}
