/**
 * @file lexer.h
 * @brief 词法分析器公共接口
 * 
 * ## 模块介绍
 * 
 * 词法分析器（Lexer）负责将输入字符串分解为词素（Token）。
 * 它是编译器前端的第一个阶段，位于语法分析之前。
 * 
 * ## 典型使用流程
 * 
 * @code
 *   Lexer lexer;
 *   
 *   // 1. 初始化
 *   lexerInit(&lexer, "2 + 3 * 4");
 *   
 *   // 2. 循环获取 token
 *   while (1) {
 *       lexerNextToken(&lexer);
 *       if (lexer.current.type == TOKEN_END) break;
 *       // 处理 token...
 *   }
 * @endcode
 * 
 * ## 设计要点
 * 
 * - 使用"预读"模式：每次调用 lexerNextToken() 获取下一个 token
 * - 当前 token 存储在 lexer.current 中
 * - 错误信息存储在 lexer.err 和 lexer.err_pos 中
 */

#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include "calculator.h"

/* ========================================================================
 * Token 类型定义
 * ======================================================================== */

/**
 * @brief Token 类型枚举
 * 
 * 表示词法分析器识别出的词素类型。
 * 
 * ## 类型说明
 * 
 *   TOKEN_NUMBER  : 数字（整数、小数、科学计数法）
 *   TOKEN_PLUS    : 加法运算符 '+'
 *   TOKEN_MINUS   : 减法运算符 '-'（也用于一元负号）
 *   TOKEN_MUL     : 乘法运算符 '*'
 *   TOKEN_DIV     : 除法运算符 '/'
 *   TOKEN_LPAREN  : 左括号 '('
 *   TOKEN_RPAREN  : 右括号 ')'
 *   TOKEN_END     : 输入结束标记
 *   TOKEN_ERROR   : 错误标记（发生词法错误时设置）
 */
typedef enum {
    TOKEN_NUMBER,    /**< 数字常量 */
    TOKEN_PLUS,      /**< 加号 + */
    TOKEN_MINUS,     /**< 减号 - */
    TOKEN_MUL,       /**< 乘号 * */
    TOKEN_DIV,       /**< 除号 / */
    TOKEN_LPAREN,    /**< 左括号 ( */
    TOKEN_RPAREN,    /**< 右括号 ) */
    TOKEN_END,       /**< 输入结束 */
    TOKEN_ERROR      /**< 错误状态 */
} TokenType;

/**
 * @brief Token 结构
 * 
 * 表示一个完整的词素，包含类型和值信息。
 * 
 * ## 字段说明
 * 
 *   - type      : Token 类型
 *   - value     : 数值（仅对 TOKEN_NUMBER 有意义）
 *   - start_pos : 词素在原字符串中的起始位置
 *   - end_pos   : 词素在原字符串中的结束位置（下一个字符的索引）
 */
typedef struct {
    TokenType type;      /**< Token 类型 */
    double value;        /**< 数值（仅数字类型使用） */
    size_t start_pos;    /**< 起始位置（字符索引） */
    size_t end_pos;      /**< 结束位置（下一个字符的索引） */
} Token;

/* ========================================================================
 * 词法分析器结构
 * ======================================================================== */

/**
 * @brief 词法分析器状态
 * 
 * 维护词法分析所需的所有状态信息。
 * 
 * ## 字段说明
 * 
 *   - input      : 输入字符串指针
 *   - length     : 输入字符串长度
 *   - pos        : 当前读取位置
 *   - current    : 当前 token
 *   - err        : 错误状态
 *   - err_pos    : 错误位置
 * 
 * ## 设计要点
 * 
 * - input 只是指针，不会复制字符串内容
 * - pos 是一个"半开区间"指针，指向下一个要读取的字符
 * - err 只设置第一次错误，避免错误信息被覆盖
 */
typedef struct {
    const char* input;   /**< 输入字符串 */
    size_t length;       /**< 字符串长度 */
    size_t pos;           /**< 当前读取位置 */
    Token current;        /**< 当前 Token */
    CalcError err;        /**< 错误状态 */
    size_t err_pos;       /**< 错误位置 */
} Lexer;

/* ========================================================================
 * 函数声明
 * ======================================================================== */

/**
 * @brief 初始化词法分析器
 * 
 * 设置输入字符串并重置所有状态。
 * 
 * @param lexer 词法分析器实例
 * @param input 要分析的输入字符串
 */
void lexerInit(Lexer* lexer, const char* input);

/**
 * @brief 获取下一个 Token
 * 
 * 读取并返回下一个词素。结果存储在 lexer->current 中。
 * 
 * 处理流程：
 *   1. 跳过空白字符
 *   2. 检查结束
 *   3. 识别字符类型
 *   4. 解析并返回 Token
 * 
 * @param lexer 词法分析器实例
 */
void lexerNextToken(Lexer* lexer);

/**
 * @brief 预扫描并打印所有 Token
 *
 * 在解析开始前调用，一次性输出所有 token。
 * 仅在 DEBUG_ENABLE=1 时可用。
 */
#ifdef DEBUG_ENABLE
void lexerDebugPrintAll(Lexer *lexer);
#endif

#endif  /* LEXER_H */
