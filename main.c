/*******************************************************************************
 * main.c - 控制台计算器程序主入口
 * 
 * 这是一个用于学习C语言的练习项目
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
 * 【学习提示】
 * 本程序采用了"递归下降解析器"（Recursive Descent Parser）算法
 * 这是一种经典且易于理解的表达式求值方法
 * 
 * 请查看 token.h 和 calculator.c 文件了解核心算法
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "calculator.h"

/*-----------------------------------------------------------------------------
 * 函数声明
 *---------------------------------------------------------------------------*/

/**
 * printWelcome - 打印欢迎信息
 * 
 * 【框架已提供】
 */
void printWelcome(void)
{
    printf("===========================================\n");
    printf("       C语言控制台计算器 v1.0\n");
    printf("===========================================\n");
    printf("\n");
    printf("支持运算: + - * / ( )\n");
    printf("输入示例: (3-5)*6 或 2+3*4\n");
    printf("输入 'quit' 或 'exit' 退出程序\n");
    printf("\n");
}

/**
 * printResult - 打印计算结果
 * 
 * 【框架已提供】
 * 
 * @param expression 用户输入的表达式
 * @param result 计算结果
 */
void printResult(const char* expression, double result)
{
    printf("表达式: %s\n", expression);
    printf("结果:   %.2f\n", result);  // 保留2位小数输出
    printf("\n");
}

/**
 * isExitCommand - 检查是否为退出命令
 * 
 * 【框架已提供】
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
 * 【框架已提供】
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
 * 
 * 【框架已提供】
 */
void printHelp(void)
{
    printf("【使用提示】\n");
    printf("- 直接输入数学表达式，例如: 2+3*4\n");
    printf("- 使用括号改变优先级，例如: (2+3)*4\n");
    printf("- 支持小数，例如: 3.14 * 2\n");
    printf("\n");
}

/*-----------------------------------------------------------------------------
 * 主函数 - 程序入口
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // 打印欢迎信息
    printWelcome();
    
    // 打印帮助
    printHelp();
    
    // 主循环：持续接收用户输入直到退出
    while (1) {
        // 提示用户输入
        printf("请输入表达式> ");
        
        // 读取用户输入（最多255个字符）
        char input[256] = {0};
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // 输入出错（如Ctrl+D/Ctrl+Z）
            printf("\n程序结束\n");
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
            printf("感谢使用，再见！\n");
            break;
        }
        
        // 计算表达式
        double result = 0.0;
        int ret = evaluate(input, &result);
        
        if (ret == 0) {
            // 计算成功，打印结果
            printResult(input, result);
        } else {
            // 计算失败
            printf("错误: 表达式计算失败，请检查输入是否正确\n");
            printf("\n");
        }
    }
    
    return 0;
}

/*-----------------------------------------------------------------------------
 * 【学习任务 - 你需要完成的函数】
 * 
 * 以下函数在 calculator.c 中标记为 TODO，你需要实现它们：
 * 
 * 1. getNextToken()      - 词法分析，识别Token
 *    提示：使用 isdigit() 检查数字，使用 switch 处理运算符
 * 
 * 2. parseFactor()       - 解析因子（数字或括号表达式）
 *    提示：处理 TOKEN_NUMBER 和 TOKEN_LPAREN
 * 
 * 3. parseTerm()        - 解析项（乘除运算）
 *    提示：使用 while 循环处理连续的乘除
 * 
 * 4. parseExpression() - 解析表达式（加减运算）
 *    提示：使用 while 循环处理连续的加减
 * 
 * 【实现顺序建议】
 * 1. 先实现 getNextToken() - 这是基础
 * 2. 再实现 parseFactor() - 处理括号
 * 3. 然后 parseTerm()     - 处理乘除
 * 4. 最后 parseExpression() - 处理加减
 * 
 * 【测试用例】
 *   输入: 2+3*4      期望输出: 14  (先乘除后加减)
 *   输入: (2+3)*4    期望输出: 20  (括号优先)
 *   输入: 10/3       期望输出: 3.33
 *   输入: (3-5)*6    期望输出: -12
 * 
 * 祝你学习愉快！🎉
 *---------------------------------------------------------------------------*/
