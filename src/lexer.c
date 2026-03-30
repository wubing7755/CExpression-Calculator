#include "lexer.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void lexerSetError(Lexer* lexer, CalcError err, size_t pos)
{
    if (lexer->err == CALC_OK) {
        lexer->err = err;
        lexer->err_pos = pos;
    }
    lexer->current.type = TOKEN_ERROR;
    lexer->current.value = 0.0;
    lexer->current.start_pos = pos;
    lexer->current.end_pos = pos;
}

static void lexerSkipWhitespace(Lexer* lexer)
{
    while (lexer->pos < lexer->length &&
           isspace((unsigned char)lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

void lexerInit(Lexer* lexer, const char* input)
{
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

void lexerNextToken(Lexer* lexer)
{
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

        if (isdigit(c) ||
            (c == '.' && start + 1 < lexer->length &&
             isdigit((unsigned char)lexer->input[start + 1]))) {
            char* endp = NULL;
            errno = 0;
            const double value = strtod(lexer->input + start, &endp);

            if (endp == lexer->input + start || errno == ERANGE || !isfinite(value)) {
                lexerSetError(lexer, CALC_ERROR_INVALID_CHAR, start);
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
