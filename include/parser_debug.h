/**
   * @file parser_debug.h
   * @brief 解析器调试系统
   *
   * ============================================================
   * 调试级别（可组合）
   * ============================================================
   *
   *   PARSER_DEBUG_NONE    = 0       : 无调试输出
   *   PARSER_DEBUG_ERROR   = (1<<0)  : 错误信息
   *   PARSER_DEBUG_TOKEN   = (1<<1)  : Token 流转
   *   PARSER_DEBUG_CALL    = (1<<2)  : 函数调用进入/退出
   *   PARSER_DEBUG_STEP    = (1<<3)  : 每步计算结果
   *
   * ============================================================
   * 使用方法
   * ============================================================
   *
   * 编译时设置（默认全部关闭）：
   *   gcc -DPARSER_DEBUG_LEVEL=3 ...  // 开启 ERROR + TOKEN
   *
   * 或在代码中设置 parser->debug_flags（运行时）
   *
   * ============================================================
   * 示例输出
   * ============================================================
   *
   * 输入 "2 + ---3 * (4 - 1)"：
   *
   *   [ENTER] parseExpression
   *     [ENTER] parseTerm
   *       [ENTER] parseUnary
   *         [ENTER] parsePrimary
   *         [EXIT]  parsePrimary => 2
   *         [OP] binary +
   *       [ENTER] parseTerm
   *         [ENTER] parseUnary
   *           [ENTER] parseUnary
   *             [ENTER] parseUnary
   *               [ENTER] parsePrimary
   *               [EXIT]  parsePrimary => 3
   *             [EXIT]  parseUnary => -3
   *           [EXIT]  parseUnary => 3
   *         [ENTER] parseTerm
   *           [ENTER] parseUnary
   *             [ENTER] parsePrimary
   *             [EXIT]  parsePrimary => 4
   *           [ENTER] parseUnary
   *             [ENTER] parsePrimary
   *             [EXIT]  parsePrimary => 1
   *           [OP] binary -
   *           [EXIT]  parseTerm => 3
   *         [EXIT]  parseUnary => 12
   */

#ifndef PARSER_DEBUG_H
#define PARSER_DEBUG_H

#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * Token 名称字符串（用于调试输出）
 * ============================================================ */

static inline const char* debugTokenName(int type) {
    switch (type) {
        case 0:  return "TOKEN_NUMBER";
        case 1:  return "TOKEN_PLUS";
        case 2:  return "TOKEN_MINUS";
        case 3:  return "TOKEN_MUL";
        case 4:  return "TOKEN_DIV";
        case 5:  return "TOKEN_LPAREN";
        case 6:  return "TOKEN_RPAREN";
        case 7:  return "TOKEN_END";
        case 8:  return "TOKEN_ERROR";
        default: return "UNKNOWN";
    }
}

/* ============================================================
 * 宏：统一使用 g_debug_level 进行运行时控制
 *
 * 级别对应：
 *   DEBUG_LEVEL_TRACE (5): Parser 函数调用树 + 中间计算结果
 *   DEBUG_LEVEL_ERROR (1): Error only
 * ============================================================ */

#if DEBUG_ENABLE
    #define PARSER_TRACE_ENTER(parser, func) \
        do { \
            if (g_debug_level >= DEBUG_LEVEL_TRACE) { \
                printf("[PARSER] %*s%-4s %s()\n", \
                    (parser)->depth * 4, "", "│  ", func); \
            } \
        } while(0)

    #define PARSER_TRACE_EXIT(parser, func, result) \
        do { \
            if (g_debug_level >= DEBUG_LEVEL_TRACE) { \
                (void)(result); \
            } \
        } while(0)

    #define PARSER_TRACE_ERROR(parser, fmt, ...) \
        do { \
            if (g_debug_level >= DEBUG_LEVEL_ERROR) { \
                printf("[PARSER] ERROR: " fmt "\n", ##__VA_ARGS__); \
            } \
        } while(0)
#else
    #define PARSER_TRACE_ENTER(parser, func) ((void)0)
    #define PARSER_TRACE_EXIT(parser, func, result) ((void)0)
    #define PARSER_TRACE_ERROR(parser, fmt, ...) ((void)0)
#endif

#endif /* PARSER_DEBUG_H */