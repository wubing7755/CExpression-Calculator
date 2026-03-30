#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>

typedef enum {
    CALC_OK = 0,
    CALC_ERROR_NULL_EXPR,
    CALC_ERROR_INVALID_CHAR,
    CALC_ERROR_DIV_BY_ZERO,
    CALC_ERROR_UNEXPECTED_TOKEN,
    CALC_ERROR_MISSING_RPAREN,
    CALC_ERROR_SYNTAX,
    CALC_ERROR_RECURSION_LIMIT
} CalcError;

CalcError evaluate(const char* expression, double* result, size_t* err_pos);
const char* calcGetErrorMessage(CalcError err);

/* Backward-compatible alias used by existing code. */
const char* parserGetErrorMessage(CalcError err);

#endif // CALCULATOR_H
