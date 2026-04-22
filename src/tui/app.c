#include "tui/app.h"

#include "tui/async.h"
#include "tui/history.h"
#include "tui/input.h"
#include "tui/layout.h"
#include "tui/response_format.h"
#include "tui/render.h"

#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int map_curses_key(int key) {
    switch (key) {
        case KEY_UP:
            return TUI_KEY_UP;
        case KEY_DOWN:
            return TUI_KEY_DOWN;
        case KEY_LEFT:
            return TUI_KEY_LEFT;
        case KEY_RIGHT:
            return TUI_KEY_RIGHT;
        case KEY_PPAGE:
            return TUI_KEY_PGUP;
        case KEY_NPAGE:
            return TUI_KEY_PGDN;
        case KEY_BACKSPACE:
            return TUI_KEY_BACKSPACE;
        case KEY_DC:
            return TUI_KEY_DELETE;
        case KEY_ENTER:
            return TUI_KEY_ENTER;
        default:
            return key;
    }
}

int tui_run(void) {
    TuiLayout layout;
    TuiHistory history;
    TuiInputState input;
    TuiAsync async_state;
    int spinner = 0;
    const char spinner_chars[] = {'|', '/', '-', '\\'};
    char status_text[160];
    int running = 1;

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fprintf(stderr, "Error: --tui requires TTY stdin/stdout.\n");
        return 1;
    }
    if (tui_async_init(&async_state) != 0) {
        fprintf(stderr, "Error: failed to start async worker.\n");
        return 1;
    }

    tui_history_init(&history);
    tui_input_init(&input);
    tui_history_append(&history, TUI_MSG_STATUS, "TUI mode ready. Enter submit, Shift+Enter newline, Esc/Ctrl+C quit.");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(80);
    curs_set(1);

    while (running) {
        int rows = 0;
        int cols = 0;
        int ch;
        TuiAsyncResult result;
        TuiInputOutcome outcome;
        char submitted[TUI_MAX_INPUT];
        int max_scroll;
        int output_width;
        int output_rows;

        getmaxyx(stdscr, rows, cols);
        tui_layout_compute(rows, cols, tui_input_line_count(&input), &layout);
        tui_input_adjust_viewport(&input, layout.input_inner_w, layout.input_inner_h);
        output_width = layout.output_inner_w > 0 ? layout.output_inner_w : 1;
        output_rows = layout.output_inner_h > 0 ? layout.output_inner_h : 1;
        max_scroll = tui_history_max_scroll_rows(&history, output_width, output_rows);
        if (history.scroll > max_scroll) {
            history.scroll = max_scroll;
        }

        if (tui_async_poll(&async_state, &result) == 1) {
            char line[TUI_MAX_MESSAGE_TEXT];
            if (result.ok) {
                if (tui_format_score_response(&result.score, line, sizeof(line)) != 0) {
                    snprintf(line, sizeof(line), "Error: failed to format response.");
                }
                tui_history_append(&history, TUI_MSG_RESULT, line);
            } else {
                tui_history_append(&history, TUI_MSG_ERROR, result.error);
            }
            history.scroll = 0;
        }

        if (tui_async_is_busy(&async_state)) {
            snprintf(status_text, sizeof(status_text), "Evaluating... %c | queue %d | %s",
                     spinner_chars[spinner % 4],
                     tui_async_pending_count(&async_state),
                     layout.compact_mode ? "compact layout" : "PgUp/PgDn + Up/Down scroll | Ctrl+L clear");
            spinner++;
        } else {
            snprintf(status_text, sizeof(status_text), "Ready | Enter submit | Shift+Enter newline | Esc/Ctrl+C quit%s",
                     layout.compact_mode ? " | compact layout" : "");
        }
        tui_render_screen(&layout, &history, &input, status_text);

        ch = getch();
        if (ch == ERR) {
            continue;
        }
        if (ch == KEY_RESIZE) {
            continue;
        }
#ifdef KEY_SENTER
        if (ch == KEY_SENTER) {
            ch = TUI_KEY_SHIFT_ENTER;
        }
#endif
        ch = map_curses_key(ch);
        if (tui_input_handle_key(&input, ch, &outcome, submitted, sizeof(submitted)) != 0) {
            tui_history_append(&history, TUI_MSG_ERROR, "Input buffer full.");
            continue;
        }

        if (outcome.action == TUI_INPUT_ACTION_QUIT) {
            running = 0;
            continue;
        }
        if (outcome.scroll_page_delta != 0) {
            int page_rows = layout.output_inner_h > 0 ? layout.output_inner_h : 1;
            tui_history_scroll_pages(&history, outcome.scroll_page_delta, page_rows, max_scroll);
        }
        if (outcome.scroll_line_delta != 0) {
            tui_history_scroll_lines(&history, outcome.scroll_line_delta, max_scroll);
        }
        if (outcome.action == TUI_INPUT_ACTION_SUBMIT) {
            tui_history_append(&history, TUI_MSG_USER, submitted);
            if (tui_async_submit(&async_state, submitted) != 0) {
                tui_history_append(&history, TUI_MSG_ERROR, "Queue full. Prompt not submitted.");
            }
            spinner = 0;
            history.scroll = 0;
        }
    }

    endwin();
    tui_async_shutdown(&async_state);
    return 0;
}
