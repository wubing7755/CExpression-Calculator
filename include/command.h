#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

typedef struct {
    bool show_process;
    bool should_exit;
} CommandState;

typedef enum {
    COMMAND_RESULT_NOT_COMMAND = 0,
    COMMAND_RESULT_HANDLED,
    COMMAND_RESULT_ERROR
} CommandResult;

void commandStateInit(CommandState* state);
CommandResult commandDispatch(const char* input, CommandState* state);

#endif
