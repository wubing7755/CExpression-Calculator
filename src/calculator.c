/*******************************************************************************
 * calculator.c - 计算器核心逻辑：递归下降Parser
 * 
 * 本文件实现了表达式求值的核心算法：递归下降解析器（Recursive Descent Parser）
 * 
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "token.h"
#include "logger.h"

double parseExpression(void);

char g_inputExpr[256] = {0};   // 用户输入的表达式
int g_pos = 0;                 // 当前读取位置
Token g_currentToken;         // 当前Token

/**
 * skipWhitespace - 跳过空白字符
 * 
 * 说明：跳过空格和制表符，更新全局位置 g_pos
 */
void skipWhitespace(void)
{
    const char* p = g_inputExpr + g_pos;

    while (*p == ' ' || *p == '\t') {
        p++;
        g_pos++;
    }
}

/**
 * getNextToken - 消费掉当前Token，并获取下一Token（词法分析）
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
    // 实现词法分析逻辑
    
    g_currentToken.type = TOKEN_ERROR;

    // 先调用 skipWhitespace 跳过空格
    skipWhitespace();

    // 获取当前字符
    const char* pStart = g_inputExpr + g_pos;
    char c = *pStart;
    
    // 字符串结束
    if (c == '\0') {
        g_currentToken.type = TOKEN_END;
        return;
    }

    // 数字 - 使用指针实现
    if (isdigit(c)) {
        char numStr[64] = {0};
        char* p = numStr;  // 指针 p 指向 numStr
        
        // 读取所有连续的数字字符和小数点
        // 使用指针算术访问字符
        while (isdigit(*(g_inputExpr + g_pos)) || *(g_inputExpr + g_pos) == '.') {
            *p++ = *(g_inputExpr + g_pos);
            g_pos++;
        }
        *p = '\0';  // 字符串结束符
        
        // 转换为 double 值
        g_currentToken.type = TOKEN_NUMBER;
        g_currentToken.value = atof(numStr);
        
        return;  // 数字已经处理完，直接返回
    }

    // 添加运算符和括号的识别
    switch (c) {
        case '+':
            g_currentToken.type = TOKEN_PLUS;
            break;
        case '-':
            g_currentToken.type = TOKEN_MINUS;
            break;
        case '*':
            g_currentToken.type = TOKEN_MUL;
            break;
        case '/':
            g_currentToken.type = TOKEN_DIV;
            break;
        case '(':
            g_currentToken.type = TOKEN_LPAREN;
            break;
        case ')':
            g_currentToken.type = TOKEN_RPAREN;
            break;
    }

    g_pos++;  // 移动到下一个字符
    return;
}

/**
 * parseFactor - 解析因子（Factor）
 * 
 *   Factor  -> ['+'|'-'] ( Number | '(' Expr ')' )
 * 
 * @return 计算结果
 */
double parseFactor(void)
{
   // 一元运算符
   int sign = 1;

   if (g_currentToken.type == TOKEN_MINUS)
   {
        getNextToken();
        sign = -1;
   }
   else if (g_currentToken.type == TOKEN_PLUS)
   {
        getNextToken();
   }
   
   if (g_currentToken.type == TOKEN_LPAREN)
   {
        getNextToken();
        double expr = parseExpression();
        getNextToken();
        return sign * expr;
   }

   if (g_currentToken.type == TOKEN_NUMBER)
    {
        double num = g_currentToken.value;
        getNextToken();
        return sign * num;
    }

    logger_log(LOG_ERROR, "Unexpected token in factor");
    return 0.0;
}

/**
 * parseTerm - 解析项（Term）
 *
 *   Term    -> Factor   | Term ('*'|'/') Factor
 * 
 * @return 计算结果
 */
double parseTerm(void)
{
    // 第一步：解析第一个因子
    double result = parseFactor();
    
    // 第二步：循环处理乘除运算
    while (g_currentToken.type == TOKEN_MUL || g_currentToken.type == TOKEN_DIV) {
        TokenType op = g_currentToken.type;
        getNextToken();
        double right = parseFactor();
        
        if (op == TOKEN_MUL) {
            result = result * right;
        } else {
            // 处理除法，除零错误
            if (right == 0.0){
                logger_log(LOG_ERROR, "Division by zero!");
                return 0.0;
            }

            result = result / right;
        }
    }
    
    return result;
}

/**
 * parseExpression - 解析表达式（Expression）
 * 
 *   Expr    -> Term     | Expr ('+'|'-') Term
 * 
 * @return 最终计算结果
 */
double parseExpression(void)
{
    // 第一步：解析第一个项
    double result = parseTerm();
    
    // 第二步：循环处理加减运算
    while (g_currentToken.type == TOKEN_PLUS || g_currentToken.type == TOKEN_MINUS) {
        TokenType op = g_currentToken.type;
        getNextToken();
        double right = parseTerm();
        
        if (op == TOKEN_PLUS) {
            result = result + right;
        } else {
            result = result - right;
        }
    }
    
    return result;
}

/**
 * evaluate - 表达式求值主函数
 * 
 * 【框架已提供】
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
    if (g_currentToken.type != TOKEN_END) {
        logger_log(LOG_WARNING, "Extra tokens after expression");
    }
    
    return 0;
}
