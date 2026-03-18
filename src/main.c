/*******************************************************************************
 * main.c - 主入口
 * 
 * 这是一个采用了"递归下降解析器"（Recursive Descent Parser）算法的表达式求值程序
 * 功能：接收用户输入的数学表达式，计算并输出结果
 * 
 * 支持的运算：
 *   - 加法 (+)
 *   - 减法 (-)
 *   - 乘法 (*)
 *   - 除法 (/)
 *   - 括号 ()
 *   - 运算符优先级
 *
 * 文法：
 *   Expr    -> Term     | Expr ('+'|'-') Term
 *   Term    -> Factor   | Term ('*'|'/') Factor
 *   Factor  -> ['+'|'-'] ( Number | '(' Expr ')' )
 *   Number  -> Digit { Digit }
 *   Digit   -> '0'|'1'|'2'|'3'|...|'9'
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "calculator.h"
#include "token.h"
#include "platform.h"
#include "logger.h"

/*-----------------------------------------------------------------------------
 * 函数声明
 *---------------------------------------------------------------------------*/

/**
 * printWelcome - 打印欢迎信息
 */
void printWelcome(void)
{
    logger_log(LOG_INFO, "===========================================\n");
    logger_log(LOG_INFO, "       C语言控制台计算器 v1.0\n");
    logger_log(LOG_INFO, "===========================================\n");
    logger_log(LOG_INFO, "\n");
    logger_log(LOG_INFO, "支持运算: + - * / ( )\n");
    logger_log(LOG_INFO, "输入示例: (3-5)*6 或 2+3*4\n");
    logger_log(LOG_INFO, "输入 'quit' 或 'exit' 退出程序\n");
    logger_log(LOG_INFO, "\n");
}

/**
 * printResult - 打印计算结果
 * 
 * @param expression 用户输入的表达式
 * @param result 计算结果
 */
void printResult(const char* expression, double result)
{
    logger_log(LOG_INFO, "表达式: %s\n", expression);
    logger_log(LOG_INFO, "结果:   %.2f\n", result);  // 保留2位小数输出
    logger_log(LOG_INFO, "\n");
}

/**
 * isExitCommand - 检查是否为退出命令
 * 
 * @param input 用户输入
 * @return true 表示退出
 */
bool isExitCommand(const char* input)
{
    // 忽略大小写比较
    if (strcmp(input, "quit") == 0 ||
        strcmp(input, "exit") == 0 ||
        strcmp(input, "q") == 0) {
        return true;
    }
    return false;
}

/**
 * isValidExpression - 简单检查表达式是否为空
 * 
 * @param input 用户输入
 * @return true 表示有效
 */
bool isValidExpression(const char* input)
{
    // 跳过空白字符
    while (*input) {
        if (!isspace(*input)) {
            return true;
        }
        input++;
    }
    return false;
}

/**
 * printHelp - 打印帮助信息
 */
void printHelp(void)
{
    logger_log(LOG_INFO, "【使用提示】\n");
    logger_log(LOG_INFO, "- 直接输入数学表达式，例如: 2+3*4\n");
    logger_log(LOG_INFO, "- 使用括号改变优先级，例如: (2+3)*4\n");
    logger_log(LOG_INFO, "- 支持小数，例如: 3.14 * 2\n");
    logger_log(LOG_INFO, "\n");
}

/*-----------------------------------------------------------------------------
 * 主函数 - 程序入口
 *---------------------------------------------------------------------------*/

int main(void)
{
    // 初始化平台
    platform_init();
    
    // 启用 UTF-8 控制台输出
    platform_enable_utf8();
    
    // 初始化日志系统，设置默认日志级别为 INFO
    logger_init(LOG_INFO);

    // 打印欢迎信息
    printWelcome();
    
    // 打印帮助
    printHelp();
    
    // 主循环：持续接收用户输入直到退出
    while (1) {
        // 提示用户输入
        logger_log(LOG_INFO, "请输入表达式> ");
        
        // 读取用户输入（最多255个字符）
        char input[256] = {0};
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // 输入中止（如Ctrl+D/Ctrl+Z）
            logger_log(LOG_INFO, "\n程序结束\n");
            break;
        }
        
        // 移除换行符
        input[strcspn(input, "\n")] = '\0';
        
        // 检查是否为空输入
        if (!isValidExpression(input)) {
            continue;
        }
        
        // 检查是否为退出命令
        if (isExitCommand(input)) {
            logger_log(LOG_INFO, "感谢使用，再见！\n");
            break;
        }
        
        // 计算表达式
        double result = 0.0;
        CalcError err = evaluate(input, &result);
        
        if (err == CALC_OK) {
            // 计算成功，打印结果
            printResult(input, result);
        } else {
            // 计算失败
            logger_log(LOG_ERROR, "错误: %s\n", parserGetErrorMessage(err));
            logger_log(LOG_INFO, "\n");
        }
    }
    
    // 清理平台资源
    platform_cleanup();
    
    return 0;
}
