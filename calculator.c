/*******************************************************************************
 * calculator.c - 计算器核心逻辑：递归下降Parser
 * 
 * 本文件实现了表达式求值的核心算法：递归下降解析器（Recursive Descent Parser）
 * 
 * 【学习目标】
 * - 理解递归下降算法的基本思想
 * - 理解运算符优先级的处理方式
 * - 理解BNF语法定义的递归性质
 * 
 * 【核心概念 - 递归下降算法】
 * 
 * 数学表达式的递归定义：
 *   表达式 = 项 { (+|-) 项 }           // 加减运算，最低优先级
 *   项     = 因子 { (*|/) 因子 }       // 乘除运算，次高优先级
 *   因子   = 数字 | '(' 表达式 ')'    // 括号或数字，最高优先级
 * 
 * 这种定义本身就是递归的！外层表达式由内层项组成，项由因子组成。
 * 通过函数调用的嵌套（递归），自然地实现了运算符优先级。
 * 
 * 示例：(3+5)*2
 *   1. parseExpression() 调用 parseTerm()
 *   2. parseTerm() 调用 parseFactor()
 *   3. parseFactor() 遇到 '('，递归调用 parseExpression() 处理 (3+5)
 *   4. 递归返回后得到 8，再处理 *2
 * 
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "token.h"

/*-----------------------------------------------------------------------------
 * 全局变量定义
 *---------------------------------------------------------------------------*/
char g_inputExpr[256] = {0};   // 用户输入的表达式
int g_pos = 0;                 // 当前读取位置
Token g_currentToken;         // 当前Token

/*-----------------------------------------------------------------------------
 * 辅助函数实现（框架已提供）
 *---------------------------------------------------------------------------*/

/**
 * skipWhitespace - 跳过空白字符
 * 
 * 说明：跳过空格和制表符，更新全局位置 g_pos
 */
void skipWhitespace(void)
{
    while (g_inputExpr[g_pos] == ' ' || g_inputExpr[g_pos] == '\t') {
        g_pos++;
    }
}

/*-----------------------------------------------------------------------------
 * 【TODO - 由你实现】
 *---------------------------------------------------------------------------*/

/**
 * getNextToken - 获取下一个Token（词法分析）
 * 
 * 【你需要实现这个函数】
 * 
 * 功能：
 * 1. 跳过空白字符
 * 2. 识别当前字符的类型，返回对应的Token
 * 
 * 算法提示：
 * - 如果是数字：读取连续的数字字符，转换为double值
 * - 如果是 + - * / ( ) ：直接返回对应Token
 * - 如果是字符串结束符 \0 ：返回 TOKEN_END
 * - 其他字符：返回 TOKEN_ERROR
 * 
 * 【学习提示】
 * - 使用 isdigit() 检查字符是否为数字
 * - 处理多位数字：用一个循环读取所有连续数字字符
 * - 处理小数点：这是可选挑战！
 * - g_pos 记录当前读取到哪个字符了
 */
void getNextToken(void)
{
    // TODO: 实现词法分析逻辑
    // 
    // 步骤1: 跳过空白字符
    //   调用 skipWhitespace();
    //
    // 步骤2: 检查当前字符
    //   if (g_inputExpr[g_pos] 是数字) 
    //       -> 解析数字，创建 TOKEN_NUMBER
    //   else if (g_inputExpr[g_pos] == '+')
    //       -> 创建 TOKEN_PLUS
    //   else if (g_inputExpr[g_pos] == '-')
    //       -> 创建 TOKEN_MINUS
    //   ... 其他运算符类似
    //
    // 步骤3: 每次处理完后，移动 g_pos 到下一个字符
    //
    // 示例代码结构：
    /*
    skipWhitespace();
    
    char c = g_inputExpr[g_pos];
    
    if (isdigit(c)) {
        // TODO: 解析数字
        // 提示：用一个循环读取连续的数字字符
        // 存储到临时字符串，然后用 atof() 转换
        char numStr[64] = {0};
        int i = 0;
        while (isdigit(g_inputExpr[g_pos])) {
            numStr[i++] = g_inputExpr[g_pos++];
        }
        g_currentToken.type = TOKEN_NUMBER;
        g_currentToken.value = atof(numStr);
    }
    else if (c == '+') {
        g_currentToken.type = TOKEN_PLUS;
        g_pos++;
    }
    // ... 其他情况类似
    */
   
    // 先调用 skipWhitespace 跳过空格
    skipWhitespace();
    
    // 获取当前字符
    char c = g_inputExpr[g_pos];
    
    // 如果是字符串结束
    if (c == '\0') {
        g_currentToken.type = TOKEN_END;
        return;
    }
    
    // TODO: 添加运算符和括号的识别
    // switch (c) { ... }
    
    // 如果都不是，标记为错误
    g_currentToken.type = TOKEN_ERROR;
}

/*-----------------------------------------------------------------------------
 * 递归下降Parser（框架 + 核心逻辑说明）
 *---------------------------------------------------------------------------*/

/**
 * parseFactor - 解析因子（Factor）
 * 
 * 【框架已提供 + TODO】
 * 
 * 因子是表达式中最基本的单元：
 *   因子 = 数字 | '(' 表达式 ')'
 * 
 * 说明：
 * - 如果当前Token是数字，直接返回其值
 * - 如果是左括号 '('，递归调用 parseExpression()，然后期望右括号 ')'
 * - 这就是处理括号优先级的核心！
 * 
 * @return 计算结果
 */
double parseFactor(void)
{
    // TODO: 检查 TOKEN_NUMBER
    //   if (g_currentToken.type == TOKEN_NUMBER) {
    //       double val = g_currentToken.value;
    //       getNextToken();  // 读取下一个Token
    //       return val;
    //   }
    
    // TODO: 检查左括号
    //   else if (g_currentToken.type == TOKEN_LPAREN) {
    //       getNextToken();  // 跳过 '('
    //       double val = parseExpression();  // 递归解析括号内的表达式
    //       // TODO: 检查是否有对应的右括号
    //       // if (g_currentToken.type == TOKEN_RPAREN) { ... }
    //       return val;
    //   }
    
    // 错误处理
    printf("Error: Unexpected token in factor\n");
    return 0.0;
}

/**
 * parseTerm - 解析项（Term）
 * 
 * 【框架已提供】
 * 
 * 项处理乘除运算：
 *   项 = 因子 { (*|/) 因子 }
 * 
 * 说明：
 * - 先解析一个因子
 * - 然后循环处理后续的乘除运算
 * - 乘除优先级高于加减，所以单独处理
 * 
 * @return 计算结果
 */
double parseTerm(void)
{
    // 第一步：解析第一个因子
    double result = parseFactor();
    
    // 第二步：循环处理乘除运算
    // TODO: 完成乘除运算循环
    // while (g_currentToken.type == TOKEN_MUL || g_currentToken.type == TOKEN_DIV) {
    //     TokenType op = g_currentToken.type;
    //     getNextToken();  // 读取下一个因子
    //     double right = parseFactor();
    //     
    //     if (op == TOKEN_MUL) {
    //         result = result * right;
    //     } else {
    //         // TODO: 处理除法，注意除零错误！
    //     }
    // }
    
    return result;
}

/**
 * parseExpression - 解析表达式（Expression）
 * 
 * 【框架已提供】
 * 
 * 表达式是递归下降的入口点：
 *   表达式 = 项 { (+|-) 项 }
 * 
 * 说明：
 * - 先解析一个项（处理乘除）
 * - 然后循环处理加减运算
 * - 因为项已经处理了乘除，所以这里只需处理加减
 * 
 * 【这就是递归下降算法的入口！main函数会调用它】
 * 
 * @return 最终计算结果
 */
double parseExpression(void)
{
    // 第一步：解析第一个项
    double result = parseTerm();
    
    // 第二步：循环处理加减运算
    // TODO: 完成加减运算循环
    // while (g_currentToken.type == TOKEN_PLUS || g_currentToken.type == TOKEN_MINUS) {
    //     TokenType op = g_currentToken.type;
    //     getNextToken();  // 读取下一个项
    //     double right = parseTerm();
    //     
    //     if (op == TOKEN_PLUS) {
    //         result = result + right;
    //     } else {
    //         result = result - right;
    //     }
    // }
    
    return result;
}

/*-----------------------------------------------------------------------------
 * 主求值函数（框架已提供）
 *---------------------------------------------------------------------------*/

/**
 * evaluate - 表达式求值主函数
 * 
 * 【框架已提供 - 不需要修改】
 * 
 * 这是计算器的入口点，按顺序执行：
 * 1. 初始化全局变量
 * 2. 获取第一个Token
 * 3. 调用 parseExpression() 进行递归求值
 * 4. 检查是否有多余的Token
 * 
 * @param expression 用户输入的表达式字符串
 * @param result 用于存储计算结果的指针
 * @return 0表示成功，非0表示出错
 */
int evaluate(const char* expression, double* result)
{
    // 复制表达式到全局变量
    strncpy(g_inputExpr, expression, 255);
    g_inputExpr[255] = '\0';
    
    // 初始化位置
    g_pos = 0;
    
    // 获取第一个Token
    getNextToken();
    
    // 调用递归下降Parser求值
    *result = parseExpression();
    
    // 检查是否所有Token都已处理完毕
    // TODO: 添加检查
    // if (g_currentToken.type != TOKEN_END) {
    //     printf("Warning: Extra tokens after expression\n");
    // }
    
    return 0;
}
