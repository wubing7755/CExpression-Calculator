/**
 * @file lexer.c
 * @brief 词法分析器实现
 */

#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator.h"
#include "debug.h"

static const char* tokenTypeName(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUM";
        case TOKEN_PLUS:   return "PLUS";
        case TOKEN_MINUS:  return "MINUS";
        case TOKEN_MUL:    return "MUL";
        case TOKEN_DIV:    return "DIV";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_END:    return "END";
        default:           return "ERROR";
    }
}

/* ========================================================================
 * 预扫描并输出所有 Token
 * ======================================================================== */

#ifdef DEBUG_ENABLE

void lexerDebugPrintAll(Lexer *lexer) {
    if (g_debug_level < DEBUG_LEVEL_TRACE) return;

    TokenType saved_type = lexer->current.type;
    size_t saved_start = lexer->current.start_pos;
    size_t saved_end = lexer->current.end_pos;
    CalcError saved_err = lexer->err;
    size_t saved_err_pos = lexer->err_pos;
    size_t saved_pos = lexer->pos;

    printf("[LEXER] ");
    lexer->pos = 0;
    lexer->err = CALC_OK;

    while (1) {
        while (lexer->pos < lexer->length &&
               isspace((unsigned char)lexer->input[lexer->pos])) {
            lexer->pos++;
        }

        if (lexer->pos >= lexer->length || lexer->input[lexer->pos] == '\0') {
            printf("%s@%zu", tokenTypeName(TOKEN_END), lexer->pos);
            break;
        }

        size_t start = lexer->pos;
        unsigned char c = (unsigned char)lexer->input[start];
        const char *name = NULL;

        switch (c) {
            case '+': name = "PLUS"; lexer->pos++; break;
            case '-': name = "MINUS"; lexer->pos++; break;
            case '*': name = "MUL"; lexer->pos++; break;
            case '/': name = "DIV"; lexer->pos++; break;
            case '(': name = "LPAREN"; lexer->pos++; break;
            case ')': name = "RPAREN"; lexer->pos++; break;
            default:
                if (isdigit(c) || (c == '.' && start + 1 < lexer->length &&
                                   isdigit((unsigned char)lexer->input[start + 1]))) {
                    char *endp = NULL;
                    strtod(lexer->input + start, &endp);
                    if (endp > lexer->input + start) {
                        name = "NUM";
                        lexer->pos = (size_t)(endp - lexer->input);
                    } else {
                        lexer->pos++;
                    }
                } else {
                    lexer->pos++;
                }
                break;
        }

        if (name) {
            printf("%s@%zu", name, start);
            printf(" | ");
        }
    }

    printf("\n");

    lexer->current.type = saved_type;
    lexer->current.start_pos = saved_start;
    lexer->current.end_pos = saved_end;
    lexer->err = saved_err;
    lexer->err_pos = saved_err_pos;
    lexer->pos = saved_pos;
}

#endif /* DEBUG_ENABLE */

/* ========================================================================
 * 错误处理
 * ======================================================================== */

static void lexerSetError(Lexer *lexer, CalcError err, size_t pos) {
    if (lexer->err == CALC_OK) {
        lexer->err = err;
        lexer->err_pos = pos;
    }
    lexer->current.type = TOKEN_ERROR;
    lexer->current.value = 0.0;
    lexer->current.start_pos = pos;
    lexer->current.end_pos = pos;
}

/* ========================================================================
 * 空白字符跳过
 * ======================================================================== */

static void lexerSkipWhitespace(Lexer *lexer) {
    while (lexer->pos < lexer->length &&
           isspace((unsigned char)lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

/* ========================================================================
 * 词法分析器初始化
 * ======================================================================== */

void lexerInit(Lexer *lexer, const char *input) {
    lexer->input = input;
    lexer->length = strlen(input);
    lexer->pos = 0;
    lexer->err = CALC_OK;
    lexer->err_pos = 0;
    lexer->current.type = TOKEN_ERROR;
    lexer->current.value = 0.0;
    lexer->current.start_pos = 0;
    lexer->current.end_pos = 0;
}

/* ========================================================================
 * 核心：词法分析 - 读取下一个 Token
 * ======================================================================== */

void lexerNextToken(Lexer *lexer) {
    if (lexer->err != CALC_OK) {
        return;
    }

    lexerSkipWhitespace(lexer);

    if (lexer->pos >= lexer->length || lexer->input[lexer->pos] == '\0') {
        lexer->current.type = TOKEN_END;
        lexer->current.value = 0.0;
        lexer->current.start_pos = lexer->pos;
        lexer->current.end_pos = lexer->pos;
        return;
    }

    {
        const size_t start = lexer->pos;
        const unsigned char c = (unsigned char)lexer->input[start];

        switch (c) {
        case '+':
            lexer->current.type = TOKEN_PLUS;
            lexer->current.value = 0.0;
            lexer->current.start_pos = start;
            lexer->current.end_pos = start + 1;
            lexer->pos++;
            return;

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

        default:
            break;
        }

        if (isdigit(c) || (c == '.' && start + 1 < lexer->length &&
                           isdigit((unsigned char)lexer->input[start + 1]))) {
            char *endp = NULL;
            errno = 0;
            const double value = strtod(lexer->input + start, &endp);

            if (endp == lexer->input + start) {
                lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
                return;
            }

            if (errno == ERANGE || !isfinite(value)) {
                lexerSetError(lexer, CALC_ERROR_NUMBER_OVERFLOW, start);
                return;
            }

            lexer->pos = (size_t)(endp - lexer->input);
            lexer->current.type = TOKEN_NUMBER;
            lexer->current.value = value;
            lexer->current.start_pos = start;
            lexer->current.end_pos = lexer->pos;
            return;
        }

        lexer->pos++;
        lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
    }
}
