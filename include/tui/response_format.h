#ifndef TUI_RESPONSE_FORMAT_H
#define TUI_RESPONSE_FORMAT_H

#include <stddef.h>

#include "scorer.h"

int tui_format_score_response(const PromptScore *score, char *out, size_t out_cap);

#endif
