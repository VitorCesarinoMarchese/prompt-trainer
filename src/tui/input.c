#include "tui/input.h"

#include <ctype.h>
#include <string.h>

static void set_outcome_none(TuiInputOutcome *outcome) {
    memset(outcome, 0, sizeof(*outcome));
    outcome->action = TUI_INPUT_ACTION_NONE;
}

void tui_input_init(TuiInputState *state) {
    memset(state, 0, sizeof(*state));
    state->history_index = -1;
    state->view_top_row = 0;
}

static void load_history_entry(TuiInputState *state, int index) {
    strncpy(state->buffer, state->history[index], TUI_MAX_INPUT - 1);
    state->buffer[TUI_MAX_INPUT - 1] = '\0';
    state->len = strlen(state->buffer);
    state->cursor = state->len;
    state->history_index = index;
}

static void push_history(TuiInputState *state, const char *submitted) {
    if (submitted[0] == '\0') {
        return;
    }
    if (state->history_count < TUI_MAX_INPUT_HISTORY) {
        strncpy(state->history[state->history_count], submitted, TUI_MAX_INPUT - 1);
        state->history[state->history_count][TUI_MAX_INPUT - 1] = '\0';
        state->history_count++;
        return;
    }
    memmove(&state->history[0], &state->history[1], sizeof(state->history[0]) * (TUI_MAX_INPUT_HISTORY - 1));
    strncpy(state->history[TUI_MAX_INPUT_HISTORY - 1], submitted, TUI_MAX_INPUT - 1);
    state->history[TUI_MAX_INPUT_HISTORY - 1][TUI_MAX_INPUT - 1] = '\0';
}

static void clear_input(TuiInputState *state) {
    state->buffer[0] = '\0';
    state->len = 0;
    state->cursor = 0;
    state->view_top_row = 0;
    state->history_index = -1;
}

static int insert_char(TuiInputState *state, char ch) {
    if (state->len + 1 >= TUI_MAX_INPUT) {
        return -1;
    }
    memmove(state->buffer + state->cursor + 1, state->buffer + state->cursor, state->len - state->cursor + 1);
    state->buffer[state->cursor] = ch;
    state->cursor++;
    state->len++;
    return 0;
}

static void handle_history_nav(TuiInputState *state, int key, TuiInputOutcome *outcome) {
    if (state->len == 0 || state->history_index != -1) {
        if (state->history_count == 0) {
            return;
        }
        if (key == TUI_KEY_UP) {
            if (state->history_index == -1) {
                load_history_entry(state, (int)state->history_count - 1);
            } else if (state->history_index > 0) {
                load_history_entry(state, state->history_index - 1);
            }
        } else {
            if (state->history_index == -1) {
                return;
            }
            if (state->history_index < (int)state->history_count - 1) {
                load_history_entry(state, state->history_index + 1);
            } else {
                clear_input(state);
            }
        }
        return;
    }

    if (key == TUI_KEY_UP) {
        outcome->scroll_line_delta = 1;
    } else {
        outcome->scroll_line_delta = -1;
    }
}

int tui_input_handle_key(TuiInputState *state, int key, TuiInputOutcome *outcome, char *submitted, size_t submitted_cap) {
    set_outcome_none(outcome);
    if (submitted_cap > 0) {
        submitted[0] = '\0';
    }

    if (key == 27 || key == 3) {
        outcome->action = TUI_INPUT_ACTION_QUIT;
        return 0;
    }

    if (key == 12) {
        clear_input(state);
        return 0;
    }

    if (key == TUI_KEY_PGUP) {
        outcome->scroll_page_delta = 1;
        return 0;
    }
    if (key == TUI_KEY_PGDN) {
        outcome->scroll_page_delta = -1;
        return 0;
    }

    if (key == TUI_KEY_UP || key == TUI_KEY_DOWN) {
        handle_history_nav(state, key, outcome);
        return 0;
    }

    if (key == TUI_KEY_LEFT) {
        if (state->cursor > 0) {
            state->cursor--;
        }
        return 0;
    }
    if (key == TUI_KEY_RIGHT) {
        if (state->cursor < state->len) {
            state->cursor++;
        }
        return 0;
    }

    if (key == TUI_KEY_BACKSPACE || key == 127 || key == 8) {
        if (state->cursor > 0 && state->len > 0) {
            memmove(state->buffer + state->cursor - 1, state->buffer + state->cursor, state->len - state->cursor + 1);
            state->cursor--;
            state->len--;
        }
        return 0;
    }

    if (key == TUI_KEY_DELETE) {
        if (state->cursor < state->len) {
            memmove(state->buffer + state->cursor, state->buffer + state->cursor + 1, state->len - state->cursor);
            state->len--;
        }
        return 0;
    }

    if (key == TUI_KEY_SHIFT_ENTER) {
        return insert_char(state, '\n');
    }

    if (key == '\n' || key == '\r' || key == TUI_KEY_ENTER) {
        if (state->len == 0) {
            return 0;
        }
        if (state->len + 1 > submitted_cap) {
            return -1;
        }
        memcpy(submitted, state->buffer, state->len);
        submitted[state->len] = '\0';
        push_history(state, submitted);
        clear_input(state);
        outcome->action = TUI_INPUT_ACTION_SUBMIT;
        return 0;
    }

    if (isprint((unsigned char)key)) {
        return insert_char(state, (char)key);
    }

    return 0;
}

int tui_input_line_count(const TuiInputState *state) {
    int lines = 1;
    size_t i;
    for (i = 0; i < state->len; ++i) {
        if (state->buffer[i] == '\n') {
            lines++;
        }
    }
    return lines;
}

int tui_input_visual_info(const TuiInputState *state, int width, TuiWrapCursorInfo *out) {
    if (state == NULL || out == NULL) {
        return -1;
    }
    return tui_textwrap_cursor_info(state->buffer, state->len, state->cursor, width, out);
}

int tui_input_wrapped_rows(const TuiInputState *state, int width) {
    if (state == NULL) {
        return 1;
    }
    return tui_textwrap_total_rows(state->buffer, state->len, width);
}

int tui_input_visual_row(const TuiInputState *state, int width, int row_index, char *out, size_t out_cap) {
    if (state == NULL) {
        return 0;
    }
    return tui_textwrap_get_row(state->buffer, state->len, width, row_index, out, out_cap);
}

void tui_input_adjust_viewport(TuiInputState *state, int width, int visible_rows) {
    TuiWrapCursorInfo visual;
    int max_top;

    if (state == NULL) {
        return;
    }
    if (visible_rows < 1) {
        visible_rows = 1;
    }
    if (tui_input_visual_info(state, width, &visual) != 0) {
        state->view_top_row = 0;
        return;
    }

    max_top = tui_input_wrapped_rows(state, width) - visible_rows;
    if (max_top < 0) {
        max_top = 0;
    }

    if (visual.cursor_row < state->view_top_row) {
        state->view_top_row = visual.cursor_row;
    } else if (visual.cursor_row >= state->view_top_row + visible_rows) {
        state->view_top_row = visual.cursor_row - visible_rows + 1;
    }

    if (state->view_top_row < 0) {
        state->view_top_row = 0;
    }
    if (state->view_top_row > max_top) {
        state->view_top_row = max_top;
    }
}

int tui_input_view_top_row(const TuiInputState *state) {
    if (state == NULL) {
        return 0;
    }
    if (state->view_top_row < 0) {
        return 0;
    }
    return state->view_top_row;
}
