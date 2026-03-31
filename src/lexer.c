/**
 * @file lexer.c
 * @brief 词法分析器实现
 *
 * ## Token 类型
 *   TOKEN_NUMBER  : 数字（整数、小数、科学计数法）
 *   TOKEN_PLUS    : 加号 +
 *   TOKEN_MINUS   : 减号 -
 *   TOKEN_MUL     : 乘号 *
 *   TOKEN_DIV     : 除号 /
 *   TOKEN_LPAREN  : 左括号 (
 *   TOKEN_RPAREN  : 右括号 )
 *   TOKEN_END     : 输入结束
 *   TOKEN_ERROR   : 错误状态
 */

#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

    /* 处理数字 */
    if (isdigit(c) || (c == '.' && start + 1 < lexer->length &&
                       isdigit((unsigned char)lexer->input[start + 1]))) {

      char *endp = NULL;
      errno = 0;

      const double value = strtod(lexer->input + start, &endp);

      /* 错误检测：解析失败 */
      if (endp == lexer->input + start) {
        lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
        return;
      }

      /* 错误检测：数值溢出（ERANGE）或结果非有限（NaN/Inf） */
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

    /* 遇到无法识别的字符 */
    lexer->pos++; /* 推进指针，避免死循环 */
    lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
  }
}