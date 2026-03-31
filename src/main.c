#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "calculator.h"
#include "command.h"
#include "debug.h"
#include "logger.h"
#include "platform.h"

#define INPUT_BUFFER_SIZE 1024U

typedef enum {
    INPUT_OK,
    INPUT_EOF,
    INPUT_TRUNCATED
} InputReadStatus;

static void printWelcome(void)
{
    logger_log(LOG_INFO, "===========================================\n");
    logger_log(LOG_INFO, "       C语言控制台计算器 v2.0\n");
    logger_log(LOG_INFO, "===========================================\n");
    logger_log(LOG_INFO, "支持运算: + - * / ( )\n");
    logger_log(LOG_INFO, "输入 'quit' 或 'exit' 退出程序\n\n");
}

static void printHelp(void)
{
    logger_log(LOG_INFO, "【使用提示】\n");
    logger_log(LOG_INFO, "- 直接输入数学表达式，例如: 2+3*4\n");
    logger_log(LOG_INFO, "- 使用括号改变优先级，例如: (2+3)*4\n");
    logger_log(LOG_INFO, "- 支持小数和科学计数法，例如: 1e-3 * 5\n\n");
    logger_log(LOG_INFO, "- 输入 /help 查看全部命令\n");
    logger_log(LOG_INFO, "- 输入 /show process 开启计算过程输出\n");
    logger_log(LOG_INFO, "- 输入 /hide process 关闭计算过程输出\n\n");
}

static InputReadStatus readInputLine(char* input, size_t capacity)
{
    size_t len;
    int ch;

    if (fgets(input, (int)capacity, stdin) == NULL) {
        return INPUT_EOF;
    }

    len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
        return INPUT_OK;
    }

    while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
        // xxx
    }
    input[capacity - 1] = '\0';
    return INPUT_TRUNCATED;
}

static bool isValidExpression(const char* input)
{
    while (*input != '\0') {
        if (!isspace((unsigned char)*input)) {
            return true;
        }
        input++;
    }
    return false;
}

static void printProcessStep(void* user_data, const CalcStepInfo* step)
{
    (void)user_data;
    if (step->elapsed_ms > 0.0) {
        logger_log(LOG_INFO, "  步骤%u: %s (耗时 %.3f ms)\n",
                   step->step_index, step->message, step->elapsed_ms);
    } else {
        logger_log(LOG_INFO, "  步骤%u: %s\n", step->step_index, step->message);
    }
}

static void printResult(const char* expression, double result)
{
    logger_log(LOG_INFO, "表达式: %s\n", expression);
    logger_log(LOG_INFO, "结果:   %.10g\n\n", result);
}

int main(int argc, char *argv[])
{
    CommandState command_state;

    /* 解析调试参数（必须在其他初始化之前） */
    debug_init(DEBUG_LEVEL_NONE, 0);
    bool debug_active = debug_parse_args(argc, argv);

    platform_init();
    platform_enable_utf8();
    logger_init(LOG_INFO);
    commandStateInit(&command_state);

    if (debug_active) {
        DEBUG_INFO("调试模式已开启，级别=%d", debug_get_level());
    }

    printWelcome();
    printHelp();

    while (true) {
        char input[INPUT_BUFFER_SIZE];
        InputReadStatus input_status;
        double result;
        size_t err_pos;
        CalcError err;

        logger_log(LOG_INFO, "请输入表达式> ");

        input_status = readInputLine(input, sizeof(input));
        if (input_status == INPUT_EOF) {
            logger_log(LOG_INFO, "\n程序结束\n");
            break;
        }
        if (input_status == INPUT_TRUNCATED) {
            logger_log(LOG_ERROR, "错误: 输入长度超过 %u 字符，请缩短表达式后重试。\n\n",
                       (unsigned)(INPUT_BUFFER_SIZE - 1U));
            continue;
        }

        if (!isValidExpression(input)) {
            continue;
        }

        /* 以 '/' 开头的输入视为命令，跳过 '/' 前缀后分发 */
        if (input[0] == '/') {
            const CommandResult cmd_result = commandDispatch(input + 1, &command_state);
            if (cmd_result != COMMAND_RESULT_NOT_COMMAND) {
                if (command_state.should_exit) {
                    logger_log(LOG_INFO, "感谢使用，再见！\n");
                    break;
                }
                /* 命令已处理（含错误提示），不进入表达式求值。 */
                continue;
            }
        }

        if (command_state.show_process) {
            CalcEvalOptions options;
            result = 0.0;
            err_pos = 0;
            logger_log(LOG_INFO, "计算过程:\n");
            calcEvalOptionsInit(&options);
            options.on_step = printProcessStep;
            // options.debug_flags = PARSER_DEBUG_CALL | PARSER_DEBUG_TOKEN;  
            err = evaluate(input, &options, &result, &err_pos);
        } else {
            result = 0.0;
            err_pos = 0;
            err = evaluate(input, NULL, &result, &err_pos);
        }

        if (err == CALC_OK) {
            printResult(input, result);
            continue;
        }

        if (err_pos < strlen(input)) {
            logger_log(LOG_ERROR, "错误: %s (位置: %zu, 附近: '%.16s')\n\n",
                       calcGetErrorMessage(err), err_pos, input + err_pos);
        } else {
            logger_log(LOG_ERROR, "错误: %s\n\n", calcGetErrorMessage(err));
        }
    }

    platform_cleanup();
    return 0;
}
