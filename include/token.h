/*******************************************************************************
 * token.h - 词法分析模块：Token 类型定义
 * 
 * 本文件定义了表达式计算器中使用的所有标记（Token）类型
 * 
 ******************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

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
 * 全局变量声明
 * 
 * 说明：
 * - inputExpr      : 用户输入的表达式字符串
 * - pos            : 当前读取位置（索引）
 * - currentToken   : 当前正在处理的 Token
 */
extern char g_inputExpr[256];   // 存储用户输入的表达式
extern int g_pos;               // 当前字符位置
extern Token g_currentToken;    // 当前 Token

/**
 * getNextToken - 获取下一个 Token
 *
 * 返回值：
 * - 解析得到的 Token（存储在 g_currentToken 中）
 */
void getNextToken(void);

/**
 * skipWhitespace - 跳过空白字符
 * 
 * 说明：
 * - 跳过空格、制表符等空白字符
 * - 更新 g_pos 位置
 */
void skipWhitespace(void);

#endif // TOKEN_H
