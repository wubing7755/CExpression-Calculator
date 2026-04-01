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

    /* 输入过长，丢弃剩余字符直到行尾 */
    while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
        /* 丢弃超出缓冲区的字符 */
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

static void printResult(const char* expression, double result)
{
    logger_log(LOG_INFO, "表达式: %s\n", expression);
    logger_log(LOG_INFO, "结果:   %.10g\n\n", result);
}

/* ========================================================================
 * 主程序
 * ======================================================================== */

/**
 * @brief 计算器 REPL 主循环
 *
 * 程序流程：
 *   1. 初始化（调试参数、平台、日志、命令状态）
 *   2. 显示欢迎信息和帮助
 *   3. 进入 REPL 循环：
 *      - 读取用户输入
 *      - 处理命令（以 '/' 开头）
 *      - 求值表达式
 *      - 输出结果或错误
 *   4. 清理并退出
 */
int main(int argc, char *argv[])
{
    CommandState command_state;

    /* -------- 初始化阶段 -------- */

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

    /* -------- REPL 循环 -------- */

    while (true) {
        char input[INPUT_BUFFER_SIZE];
        InputReadStatus input_status;
        double result;
        size_t err_pos;
        CalcError err;

        logger_log(LOG_INFO, "请输入表达式> ");

        input_status = readInputLine(input, sizeof(input));

        /* 处理输入结束（Ctrl+D 或 Ctrl+Z） */
        if (input_status == INPUT_EOF) {
            logger_log(LOG_INFO, "\n程序结束\n");
            break;
        }

        /* 处理输入过长 */
        if (input_status == INPUT_TRUNCATED) {
            logger_log(LOG_ERROR, "错误: 输入长度超过 %u 字符，请缩短表达式后重试。\n\n",
                       (unsigned)(INPUT_BUFFER_SIZE - 1U));
            continue;
        }

        /* 跳过空输入 */
        if (!isValidExpression(input)) {
            continue;
        }

        /* -------- 命令处理 -------- */
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

        /* -------- 表达式求值 -------- */

        result = 0.0;
        err_pos = 0;
        err = evaluate(input, NULL, &result, &err_pos);

        /* -------- 结果输出 -------- */

        if (err == CALC_OK) {
            printResult(input, result);
            continue;
        }

        /* 输出错误信息 */
        if (err_pos < strlen(input)) {
            logger_log(LOG_ERROR, "错误: %s (位置: %zu, 附近: '%.16s')\n\n",
                       calcGetErrorMessage(err), err_pos, input + err_pos);
        } else {
            logger_log(LOG_ERROR, "错误: %s\n\n", calcGetErrorMessage(err));
        }
    }

    /* -------- 清理阶段 -------- */

    platform_cleanup();
    return 0;
}
