#ifndef TUI_STATE_H
#define TUI_STATE_H

#include <stddef.h>

#define TUI_MAX_MESSAGES 512
#define TUI_MAX_MESSAGE_TEXT 1024
#define TUI_MAX_INPUT 2048
#define TUI_MAX_INPUT_HISTORY 100

typedef enum {
    TUI_MSG_USER = 0,
    TUI_MSG_RESULT = 1,
    TUI_MSG_STATUS = 2,
    TUI_MSG_ERROR = 3
} TuiMessageType;

typedef struct {
    TuiMessageType type;
    char text[TUI_MAX_MESSAGE_TEXT];
} TuiMessage;

#endif
