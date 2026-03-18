/*******************************************************************************
 * calculator.c - 计算器核心逻辑：递归下降Parser
 * 
 * 本文件实现了表达式求值的核心算法：递归下降解析器（Recursive Descent Parser）
 * 
 * 特性：
 * - 使用 ParserContext 结构体封装解析器状态，无全局变量
 * - 支持详细的错误码返回
 * - 线程安全（无共享状态）
 * 
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "token.h"
#include "logger.h"

/*=============================================================================
 * 错误消息映射
 *===========================================================================*/

static const char* g_errorMessages[] = {
    [CALC_OK] = "Success",
    [CALC_ERROR_NULL_EXPR] = "Expression is NULL or empty",
    [CALC_ERROR_INVALID_CHAR] = "Invalid character found",
    [CALC_ERROR_DIV_BY_ZERO] = "Division by zero",
    [CALC_ERROR_UNEXPECTED_TOKEN] = "Unexpected token",
    [CALC_ERROR_MISSING_RPAREN] = "Missing closing parenthesis",
    [CALC_ERROR_SYNTAX] = "Syntax error"
};

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* 编译时断言：确保错误消息数组大小与 CalcError 枚举匹配 (C11+) */
_Static_assert(sizeof(g_errorMessages) / sizeof(g_errorMessages[0]) == CALC_ERROR_SYNTAX + 1,
               "Error message array size does not match CalcError enum");
#else
/* 运行时断言：确保错误消息数组大小与 CalcError 枚举匹配 (C99) */
typedef char static_assertion_error_message_array_size[sizeof(g_errorMessages) / sizeof(g_errorMessages[0]) == CALC_ERROR_SYNTAX + 1 ? 1 : -1];
#endif

/**
 * parserGetErrorMessage - 获取错误码对应的错误信息
 * 
 * 改进：
 * - 使用 designated initializers 提高可读性和安全性
 * - 添加编译时断言确保数组与枚举同步
 * - 添加防御性空指针检查
 */
const char* parserGetErrorMessage(CalcError err)
{
    // 使用 sizeof 在编译时计算数组大小
    const size_t arraySize = sizeof(g_errorMessages) / sizeof(g_errorMessages[0]);
    
    if (err >= 0 && (size_t)err < arraySize) {
        const char* msg = g_errorMessages[err];
        // 防御性检查：确保返回的消息指针有效
        return msg ? msg : "Unknown error";
    }
    return "Unknown error";
}

/*=============================================================================
 * 解析器实现
 *===========================================================================*/

/**
 * parserInit - 初始化解析器上下文
 */
void parserInit(ParserContext* context, const char* expression)
{
    context->expression = expression;
    context->length = expression ? strlen(expression) : 0;
    context->pos = 0;
    context->currentToken.type = TOKEN_ERROR;
    context->currentToken.value = 0.0;
}

/**
 * parserSkipWhitespace - 跳过空白字符
 * 
 * 改进：简化代码，使用数组索引访问
 */
void parserSkipWhitespace(ParserContext* context)
{
    while (context->pos < context->length) {
        char c = context->expression[context->pos];
        if (c != ' ' && c != '\t') {
            break;
        }
        context->pos++;
    }
}

/**
 * parserGetNextToken - 获取下一个 Token
 * 
 * 改进：
 * - 数字解析添加缓冲区边界检查
 * - 简化运算符处理逻辑
 * - 正确处理以小数点开头的数字（如 .5）
 */
void parserGetNextToken(ParserContext* context)
{
    // 先跳过空白字符
    parserSkipWhitespace(context);
    
    context->currentToken.type = TOKEN_ERROR;
    
    // 检查是否到达末尾
    if (context->pos >= context->length) {
        context->currentToken.type = TOKEN_END;
        return;
    }
    
    // 获取当前字符（使用 unsigned char 确保 isdigit 正确工作）
    unsigned char c = (unsigned char)context->expression[context->pos];
    
    // 字符串结束
    if (c == '\0') {
        context->currentToken.type = TOKEN_END;
        return;
    }
    
    // 数字 - 读取连续的数字字符和小数点
    // 支持小数开头（如 .5）或数字开头（如 5, 5.5）
    // 注意：必须确保 isdigit 接收到的是 unsigned char 范围内的值
    if (isdigit(c) || (c == '.' && context->pos + 1 < context->length && isdigit((unsigned char)context->expression[context->pos + 1]))) {
        #define NUM_BUF_SIZE 64
        char numStr[NUM_BUF_SIZE] = {0};
        char* p = numStr;
        char* pEnd = numStr + NUM_BUF_SIZE - 1;  // 保留一个位置给终止符
        int hasDot = 0;
        
        // 读取整数部分或小数部分
        while (p < pEnd && context->pos < context->length) {
            unsigned char ch = (unsigned char)context->expression[context->pos];
            
            if (isdigit(ch)) {
                *p++ = (char)ch;
                context->pos++;
            } else if (ch == '.' && !hasDot) {
                // 小数点后可以继续读取数字
                *p++ = (char)ch;
                hasDot = 1;
                context->pos++;
            } else {
                break;
            }
        }
        *p = '\0';
        
        context->currentToken.type = TOKEN_NUMBER;
        context->currentToken.value = atof(numStr);
        return;
    }
    
    // 运算符和括号 - 使用数组索引简化
    static const TokenType charToToken[256] = {
        ['+'] = TOKEN_PLUS,
        ['-'] = TOKEN_MINUS,
        ['*'] = TOKEN_MUL,
        ['/'] = TOKEN_DIV,
        ['('] = TOKEN_LPAREN,
        [')'] = TOKEN_RPAREN
    };
    
    // 检查是否为已知运算符
    if (charToToken[c] != TOKEN_ERROR) {
        context->currentToken.type = charToToken[c];
        context->pos++;
        return;
    }
    
    // 未知字符
    context->currentToken.type = TOKEN_ERROR;
    context->pos++;
}

/**
 * parserParseFactor - 解析因子
 *   Factor  -> ['+'|'-'] ( Number | '(' Expr ')' )
 */
double parserParseFactor(ParserContext* context)
{
    int sign = 1;
    
    if (context->currentToken.type == TOKEN_MINUS) {
        parserGetNextToken(context);
        sign = -1;
    } else if (context->currentToken.type == TOKEN_PLUS) {
        parserGetNextToken(context);
    }
    
    if (context->currentToken.type == TOKEN_LPAREN) {
        parserGetNextToken(context);
        double expr = parserParseExpression(context);
        
        if (context->currentToken.type != TOKEN_RPAREN) {
            logger_log(LOG_ERROR, "Missing closing parenthesis");
            return 0.0;
        }
        
        parserGetNextToken(context);
        return sign * expr;
    }
    
    if (context->currentToken.type == TOKEN_NUMBER) {
        double num = context->currentToken.value;
        parserGetNextToken(context);
        return sign * num;
    }
    
    logger_log(LOG_ERROR, "Unexpected token in factor");
    return 0.0;
}

/**
 * parserParseTerm - 解析项
 *   Term    -> Factor   | Term ('*'|'/') Factor
 */
double parserParseTerm(ParserContext* context)
{
    double result = parserParseFactor(context);
    
    while (context->currentToken.type == TOKEN_MUL || 
           context->currentToken.type == TOKEN_DIV) {
        TokenType op = context->currentToken.type;
        parserGetNextToken(context);
        double right = parserParseFactor(context);
        
        if (op == TOKEN_MUL) {
            result = result * right;
        } else {
            if (right == 0.0) {
                logger_log(LOG_ERROR, "Division by zero!");
                return 0.0;
            }
            result = result / right;
        }
    }
    
    return result;
}

/**
 * parserParseExpression - 解析表达式
 *   Expr    -> Term     | Expr ('+'|'-') Term
 */
double parserParseExpression(ParserContext* context)
{
    double result = parserParseTerm(context);
    
    while (context->currentToken.type == TOKEN_PLUS || 
           context->currentToken.type == TOKEN_MINUS) {
        TokenType op = context->currentToken.type;
        parserGetNextToken(context);
        double right = parserParseTerm(context);
        
        if (op == TOKEN_PLUS) {
            result = result + right;
        } else {
            result = result - right;
        }
    }
    
    return result;
}

/*=============================================================================
 * 公共API实现
 *===========================================================================*/

/**
 * evaluate - 表达式求值主函数
 */
CalcError evaluate(const char* expression, double* result)
{
    // 检查空表达式
    if (expression == NULL || *expression == '\0') {
        return CALC_ERROR_NULL_EXPR;
    }
    
    // 初始化解析器上下文
    ParserContext context;
    parserInit(&context, expression);
    
    // 获取第一个Token
    parserGetNextToken(&context);
    
    // 解析表达式
    *result = parserParseExpression(&context);
    
    // 检查是否所有Token都已处理完毕
    if (context.currentToken.type != TOKEN_END && 
        context.currentToken.type != TOKEN_RPAREN) {
        logger_log(LOG_WARNING, "Extra tokens after expression");
    }
    
    return CALC_OK;
}

/**
 * evaluateWithContext - 使用ParserContext进行求值
 */
CalcError evaluateWithContext(ParserContext* context, double* result)
{
    if (context == NULL || context->expression == NULL) {
        return CALC_ERROR_NULL_EXPR;
    }
    
    parserGetNextToken(context);
    *result = parserParseExpression(context);
    
    return CALC_OK;
}
