#ifndef TUI_RENDER_H
#define TUI_RENDER_H

#include "tui/history.h"
#include "tui/input.h"
#include "tui/layout.h"

void tui_render_screen(const TuiLayout *layout, const TuiHistory *history, const TuiInputState *input, const char *status_text);

#endif
