#ifndef TUI_INPUT_H
#define TUI_INPUT_H

#include <stddef.h>

#include "tui/state.h"
#include "tui/textwrap.h"

enum {
    TUI_KEY_UP = 1001,
    TUI_KEY_DOWN = 1002,
    TUI_KEY_LEFT = 1003,
    TUI_KEY_RIGHT = 1004,
    TUI_KEY_PGUP = 1005,
    TUI_KEY_PGDN = 1006,
    TUI_KEY_BACKSPACE = 1007,
    TUI_KEY_DELETE = 1008,
    TUI_KEY_SHIFT_ENTER = 1009,
    TUI_KEY_ENTER = 1010
};

typedef enum {
    TUI_INPUT_ACTION_NONE = 0,
    TUI_INPUT_ACTION_SUBMIT = 1,
    TUI_INPUT_ACTION_QUIT = 2
} TuiInputAction;

typedef struct {
    TuiInputAction action;
    int scroll_line_delta;
    int scroll_page_delta;
} TuiInputOutcome;

typedef struct {
    char buffer[TUI_MAX_INPUT];
    size_t len;
    size_t cursor;
    char history[TUI_MAX_INPUT_HISTORY][TUI_MAX_INPUT];
    size_t history_count;
    int history_index;
} TuiInputState;

void tui_input_init(TuiInputState *state);
int tui_input_handle_key(TuiInputState *state, int key, TuiInputOutcome *outcome, char *submitted, size_t submitted_cap);
int tui_input_line_count(const TuiInputState *state);
int tui_input_visual_info(const TuiInputState *state, int width, TuiWrapCursorInfo *out);

#endif
