/*******************************************************************************
 * token.h - 词法分析模块：Token 类型定义
 * 
 * 本文件定义了表达式计算器中使用的所有标记（Token）类型
 * 
 ******************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

/**
 * Token 类型枚举
 * 
 * 说明：
 * - TOKEN_NUMBER : 数字（如 123, 45.67）
 * - TOKEN_PLUS   : 加法运算符 (+)
 * - TOKEN_MINUS  : 减法运算符 (-)
 * - TOKEN_MUL    : 乘法运算符 (*)
 * - TOKEN_DIV    : 除法运算符 (/)
 * - TOKEN_LPAREN : 左括号 (()
 * - TOKEN_RPAREN : 右括号 ())
 * - TOKEN_END    : 表达式结束标记
 * - TOKEN_ERROR  : 错误标记
 */
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

/**
 * 错误码枚举
 */
typedef enum {
    CALC_OK = 0,                    // 计算成功
    CALC_ERROR_NULL_EXPR,           // 表达式为空
    CALC_ERROR_INVALID_CHAR,        // 无效字符
    CALC_ERROR_DIV_BY_ZERO,         // 除零错误
    CALC_ERROR_UNEXPECTED_TOKEN,    // 意外的Token
    CALC_ERROR_MISSING_RPAREN,      // 缺少右括号
    CALC_ERROR_SYNTAX               // 语法错误
} CalcError;

/**
 * Token 结构体
 * 
 * 作用：存储一个完整的标记信息
 * - type  : 标记的类型（TokenType）
 * - value : 如果是数字，存储数值；否则未使用
 */
typedef struct {
    TokenType type;    // Token 类型
    double value;      // 数值（仅当 type == TOKEN_NUMBER 时有效）
} Token;

/**
 * ParserContext - 解析器上下文结构体
 * 
 * 说明：
 * - expression  : 用户输入的表达式字符串
 * - length      : 表达式长度
 * - pos         : 当前读取位置（索引）
 * - currentToken: 当前正在处理的 Token
 */
typedef struct {
    const char* expression;  // 表达式字符串（只读）
    size_t length;           // 表达式长度
    size_t pos;              // 当前字符位置
    Token currentToken;      // 当前 Token
    CalcError err;           // 错误标志
    size_t err_pos;          // 错误位置
} ParserContext;

/*=============================================================================
 * 函数声明
 *===========================================================================*/

/**
 * parserInit - 初始化解析器上下文
 * 
 * @param context 解析器上下文指针
 * @param expression 要解析的表达式字符串
 */
void parserInit(ParserContext* context, const char* expression);

/**
 * parserGetNextToken - 获取下一个 Token
 * 
 * @param context 解析器上下文指针
 */
void parserGetNextToken(ParserContext* context);

/**
 * parserSkipWhitespace - 跳过空白字符
 * 
 * @param context 解析器上下文指针
 */
void parserSkipWhitespace(ParserContext* context);

/**
 * 解析函数声明
 */
double parserParseExpression(ParserContext* context);
double parserParseTerm(ParserContext* context);
double parserParseFactor(ParserContext* context);

/**
 * parserGetErrorMessage - 获取错误码对应的错误信息
 * 
 * @param err 错误码
 * @return 错误信息字符串
 */
const char* parserGetErrorMessage(CalcError err);

#endif // TOKEN_H
