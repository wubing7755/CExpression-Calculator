/**
 * @file lexer.c
 * @brief 词法分析器实现 - 将输入字符串转换为 token 流
 * 
 * ## 什么是词法分析？
 * 词法分析（Lexical Analysis）是编译原理中的第一步。它的任务是将原始输入字符串
 * 分解成一个个有意义的"词素"（Token）。例如：
 *   输入 "2 + 3 * 4" 会被分解为：数字(2), 加号(+), 数字(3), 乘号(*), 数字(4)
 * 
 * ## 设计思路
 * 本词法分析器采用"指针推进"模式：
 *   1. 维护一个位置指针 (pos) 指向当前读取位置
 *   2. 每次调用 lexerNextToken() 跳到下一个词素
 *   3. 将结果存储在 Lexer 结构体的 current 字段中
 * 
 * 这种设计的优点：
 *   - 简单直观，易于理解和实现
 *   - 支持"预读"一个 token（用于语法分析时的决策）
 *   - 错误恢复相对简单
 */

#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 错误处理机制
 * ======================================================================== */

/**
 * @brief 设置词法分析器错误状态
 * 
 * 当遇到非法字符时调用此函数。
 * 注意：只设置第一次错误，后续错误会被忽略（避免错误信息被覆盖）
 * 
 * @param lexer 词法分析器实例
 * @param err 错误类型
 * @param pos 错误发生的位置
 */
static void lexerSetError(Lexer* lexer, CalcError err, size_t pos)
{
    /* 只有在没有错误时才设置，避免第一次错误被覆盖 */
    if (lexer->err == CALC_OK) {
        lexer->err = err;
        lexer->err_pos = pos;
    }
    /* 将当前 token 标记为错误状态 */
    lexer->current.type = TOKEN_ERROR;
    lexer->current.value = 0.0;
    lexer->current.start_pos = pos;
    lexer->current.end_pos = pos;
}

/* ========================================================================
 * 空白字符跳过
 * ======================================================================== */

/**
 * @brief 跳过输入中的空白字符
 * 
 * 空白字符包括：空格、制表符(\t)、换行符(\n)、回车符(\r)等
 * 这些字符在数学表达式中没有意义，应该被忽略
 * 
 * @param lexer 词法分析器实例
 */
static void lexerSkipWhitespace(Lexer* lexer)
{
    /* 循环直到遇到非空白字符或到达字符串末尾 */
    while (lexer->pos < lexer->length &&
           isspace((unsigned char)lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

/* ========================================================================
 * 词法分析器初始化
 * ======================================================================== */

/**
 * @brief 初始化词法分析器
 * 
 * 将字符串指针存储到 Lexer 结构中，并重置所有状态。
 * 注意：这里只是存储指针，不会复制字符串内容。
 * 
 * @param lexer 词法分析器实例
 * @param input 要分析的输入字符串
 */
void lexerInit(Lexer* lexer, const char* input)
{
    lexer->input = input;                          /* 保存输入字符串指针 */
    lexer->length = strlen(input);                 /* 计算字符串长度 */
    lexer->pos = 0;                                /* 重置位置指针到开头 */
    lexer->err = CALC_OK;                         /* 清除错误状态 */
    lexer->err_pos = 0;                            /* 清除错误位置 */
    lexer->current.type = TOKEN_ERROR;             /* 初始化当前 token 为错误状态 */
    lexer->current.value = 0.0;
    lexer->current.start_pos = 0;
    lexer->current.end_pos = 0;
}

/* ========================================================================
 * 核心：词法分析 - 读取下一个 Token
 * ======================================================================== */

/**
 * @brief 获取下一个词素（Token）
 * 
 * 这是词法分析器的核心函数。它执行以下步骤：
 *   1. 跳过空白字符
 *   2. 检查是否到达字符串末尾
 *   3. 识别当前字符的类型（数字、运算符、括号）
 *   4. 解析并返回相应的 Token
 * 
 * ## Token 类型说明
 *   - TOKEN_NUMBER  : 数字（整数、小数、科学计数法如 1e-3）
 *   - TOKEN_PLUS    : 加号 (+)
 *   - TOKEN_MINUS   : 减号 (-)
 *   - TOKEN_MUL     : 乘号 (*)
 *   - TOKEN_DIV     : 除号 (/)
 *   - TOKEN_LPAREN  : 左括号 (()
 *   - TOKEN_RPAREN  : 右括号 ())
 *   - TOKEN_END     : 输入结束
 *   - TOKEN_ERROR   : 错误状态
 * 
 * ## 为什么用 switch-case？
 * 因为我们需要根据当前字符的类型做不同处理。
 * 这是一种"字符分类"的方法，比用 if-else 更清晰。
 * 
 * @param lexer 词法分析器实例
 */
void lexerNextToken(Lexer* lexer)
{
    /* 如果之前已经有错误，直接返回，不继续分析 */
    if (lexer->err != CALC_OK) {
        return;
    }

    /* 步骤1: 跳过空白字符 */
    lexerSkipWhitespace(lexer);

    /* 步骤2: 检查是否到达字符串末尾
     * 如果 pos >= length，说明已经分析完所有字符
     */
    if (lexer->pos >= lexer->length || lexer->input[lexer->pos] == '\0') {
        lexer->current.type = TOKEN_END;           /* 返回结束标记 */
        lexer->current.value = 0.0;
        lexer->current.start_pos = lexer->pos;
        lexer->current.end_pos = lexer->pos;
        return;
    }

    /* 步骤3: 识别并处理当前字符
     * 
     * 设计要点：
     *   - 记录 start 位置用于错误报告
     *   - 将字符指针前移（pos++），为下次调用做准备
     *   - 设置 token 的类型和位置信息
     */
    {
        const size_t start = lexer->pos;          /* 记录 token 开始位置 */
        const unsigned char c = (unsigned char)lexer->input[start]; /* 获取当前字符 */

        /* switch-case 状态机：根据字符类型返回不同 token */
        switch (c) {
            case '+':
                lexer->current.type = TOKEN_PLUS;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;                     /* 推进到下一个字符 */
                return;                           /* 立即返回，完成本次分析 */
            
            case '-':
                lexer->current.type = TOKEN_MINUS;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;
                return;
            
            case '*':
                lexer->current.type = TOKEN_MUL;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;
                return;
            
            case '/':
                lexer->current.type = TOKEN_DIV;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;
                return;
            
            case '(':
                lexer->current.type = TOKEN_LPAREN;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;
                return;
            
            case ')':
                lexer->current.type = TOKEN_RPAREN;
                lexer->current.value = 0.0;
                lexer->current.start_pos = start;
                lexer->current.end_pos = start + 1;
                lexer->pos++;
                return;
            
            /* 如果不是运算符或括号，跳到 default 分支处理数字 */
            default:
                break;
        }

        /* ---------------------------------------------------------------
         * 步骤4: 处理数字
         * ---------------------------------------------------------------
         * 
         * 数字格式支持：
         *   - 整数: 123
         *   - 小数: 3.14
         *   - 科学计数法: 1e-3, 2.5E+10
         * 
         * 这里使用 strtod() 库函数来解析，它能自动处理：
         *   - 整数和小数部分
         *   - 可选的小数点
         *   - 可选的指数部分（e/E 后跟正负号和数字）
         * 
         * 解析技巧：
         *   - start 指向数字第一个字符
         *   - strtod() 返回解析后的值，并通过 endp 参数返回"停止解析"的位置
         *   - 我们用 endp - input 计算出数字在原字符串中占用的长度
         *   - 然后更新 pos 指针，指向数字之后的下一个字符
         */
        if (isdigit(c) ||                                        /* 以数字开头 */
            (c == '.' && start + 1 < lexer->length &&             /* 或者以小数点开头 */
             isdigit((unsigned char)lexer->input[start + 1]))) { /* 且下一个字符是数字（防止 ".abc" 被误认为数字）*/
            
            char* endp = NULL;                                   /* strtod 会设置此指针 */
            errno = 0;                                            /* 清除之前的 errno */
            
            /* 调用 strtod 解析数字
             * 参数说明：
             *   - input + start: 从数字开始处解析
             *   - &endp: 输出参数，返回"第一个无法解析的字符"的位置
             */
            const double value = strtod(lexer->input + start, &endp);

            /* 错误检测
             * 
             * 三种错误情况：
             * 1. endp == input + start: strtod 没有解析任何字符（不可能发生，因为我们检查了 isdigit）
             * 2. errno == ERANGE: 数值超出 double 能表示的范围（溢出或下溢）
             * 3. !isfinite(value): 结果不是有限的数（NaN 或无穷大）
             */
            if (endp == lexer->input + start || errno == ERANGE || !isfinite(value)) {
                lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
                return;
            }

            /* 成功解析，更新位置和 token 信息 */
            lexer->pos = (size_t)(endp - lexer->input);           /* 推进到数字之后的字符 */
            lexer->current.type = TOKEN_NUMBER;
            lexer->current.value = value;
            lexer->current.start_pos = start;
            lexer->current.end_pos = lexer->pos;
            return;
        }

        /* ---------------------------------------------------------------
         * 步骤5: 遇到无法识别的字符
         * ---------------------------------------------------------------
         * 如果走到这里，说明当前字符不是：
         *   - 空白字符（已跳过）
         *   - 运算符 (+ - * /)
         *   - 括号 ( )
         *   - 数字或小数点
         * 
         * 这是一个非法字符，设置错误并返回。
         */
        lexer->pos++;                                             /* 推进指针，避免死循环 */
        lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
    }
}
