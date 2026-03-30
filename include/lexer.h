#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include "calculator.h"

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

typedef struct {
    TokenType type;
    double value;
    size_t start_pos;
    size_t end_pos;
} Token;

typedef struct {
    const char* input;
    size_t length;
    size_t pos;
    Token current;
    CalcError err;
    size_t err_pos;
} Lexer;

void lexerInit(Lexer* lexer, const char* input);
void lexerNextToken(Lexer* lexer);

#endif
