#include "command.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "logger.h"

#define COMMAND_INPUT_COPY_MAX 1024U
#define COMMAND_MAX_TOKENS 8U

typedef void (*CommandHandler)(CommandState* state);

typedef struct {
    const char* syntax;
    const char* description;
    const char* const* tokens;
    size_t token_count;
    CommandHandler handler;
} CommandSpec;

static int equalsIgnoreCase(const char* a, const char* b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static size_t tokenize(char* text, char* tokens[], size_t max_tokens)
{
    size_t count = 0;
    char* p = text;

    while (*p != '\0' && count < max_tokens) {
        while (*p != '\0' && isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        tokens[count++] = p;
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    return count;
}

static int isCandidateCommand(const char* token)
{
    return (token != NULL && isalpha((unsigned char)token[0]));
}

static void handleShowProcess(CommandState* state)
{
    state->show_process = true;
    logger_log(LOG_INFO, "已开启计算过程输出。\n\n");
}

static void handleHideProcess(CommandState* state)
{
    state->show_process = false;
    logger_log(LOG_INFO, "已关闭计算过程输出。\n\n");
}

static void handleQuit(CommandState* state)
{
    state->should_exit = true;
}

static void handleShowHelp(CommandState* state);

static const char* TOK_SHOW_PROCESS[] = {"show", "process"};
static const char* TOK_HIDE_PROCESS[] = {"hide", "process"};
static const char* TOK_SHOW_HELP[] = {"show", "help"};
static const char* TOK_HELP[] = {"help"};
static const char* TOK_QUIT[] = {"quit"};
static const char* TOK_EXIT[] = {"exit"};
static const char* TOK_Q[] = {"q"};

static const CommandSpec COMMAND_SPECS[] = {
    {"show process", "开启计算过程输出", TOK_SHOW_PROCESS, 2U, handleShowProcess},
    {"hide process", "关闭计算过程输出", TOK_HIDE_PROCESS, 2U, handleHideProcess},
    {"show help", "显示命令帮助", TOK_SHOW_HELP, 2U, handleShowHelp},
    {"help", "显示命令帮助", TOK_HELP, 1U, handleShowHelp},
    {"quit", "退出程序", TOK_QUIT, 1U, handleQuit},
    {"exit", "退出程序", TOK_EXIT, 1U, handleQuit},
    {"q", "退出程序", TOK_Q, 1U, handleQuit}
};

static void handleShowHelp(CommandState* state)
{
    size_t i;
    (void)state;

    logger_log(LOG_INFO, "【命令列表】\n");
    for (i = 0; i < sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0]); ++i) {
        logger_log(LOG_INFO, "- %-13s : %s\n",
                   COMMAND_SPECS[i].syntax, COMMAND_SPECS[i].description);
    }
    logger_log(LOG_INFO, "\n");
}

void commandStateInit(CommandState* state)
{
    state->show_process = false;
    state->should_exit = false;
}

CommandResult commandDispatch(const char* input, CommandState* state)
{
    char copy[COMMAND_INPUT_COPY_MAX];
    char* tokens[COMMAND_MAX_TOKENS];
    size_t token_count;
    size_t i;

    if (input == NULL || state == NULL) {
        return COMMAND_RESULT_ERROR;
    }

    copy[0] = '\0';
    strncpy(copy, input, sizeof(copy) - 1U);
    copy[sizeof(copy) - 1U] = '\0';

    token_count = tokenize(copy, tokens, COMMAND_MAX_TOKENS);
    if (token_count == 0) {
        return COMMAND_RESULT_NOT_COMMAND;
    }

    if (!isCandidateCommand(tokens[0])) {
        return COMMAND_RESULT_NOT_COMMAND;
    }

    for (i = 0; i < sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0]); ++i) {
        const CommandSpec* spec = &COMMAND_SPECS[i];
        size_t j;
        int match = 1;

        if (token_count != spec->token_count) {
            continue;
        }

        for (j = 0; j < spec->token_count; ++j) {
            if (!equalsIgnoreCase(tokens[j], spec->tokens[j])) {
                match = 0;
                break;
            }
        }

        if (match) {
            spec->handler(state);
            return COMMAND_RESULT_HANDLED;
        }
    }

    logger_log(LOG_ERROR, "未知命令: %s\n", input);
    logger_log(LOG_INFO, "输入 'show help' 查看可用命令。\n\n");
    return COMMAND_RESULT_ERROR;
}
