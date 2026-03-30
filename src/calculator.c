#include "calculator.h"

#include "parser.h"

static const char* g_error_messages[] = {
    [CALC_OK] = "Success",
    [CALC_ERROR_NULL_EXPR] = "Expression is NULL or empty",
    [CALC_ERROR_INVALID_CHAR] = "Invalid character found",
    [CALC_ERROR_DIV_BY_ZERO] = "Division by zero",
    [CALC_ERROR_UNEXPECTED_TOKEN] = "Unexpected token",
    [CALC_ERROR_MISSING_RPAREN] = "Missing closing parenthesis",
    [CALC_ERROR_SYNTAX] = "Syntax error",
    [CALC_ERROR_RECURSION_LIMIT] = "Expression nesting is too deep"
};

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(g_error_messages) / sizeof(g_error_messages[0]) == CALC_ERROR_RECURSION_LIMIT + 1,
               "Error message array size does not match CalcError enum");
#endif

const char* calcGetErrorMessage(CalcError err)
{
    const size_t count = sizeof(g_error_messages) / sizeof(g_error_messages[0]);
    if (err >= 0 && (size_t)err < count && g_error_messages[err] != NULL) {
        return g_error_messages[err];
    }
    return "Unknown error";
}

const char* parserGetErrorMessage(CalcError err)
{
    return calcGetErrorMessage(err);
}

CalcError evaluate(const char* expression, double* result, size_t* err_pos)
{
    if (expression == NULL || *expression == '\0' || result == NULL) {
        if (err_pos != NULL) {
            *err_pos = 0;
        }
        return CALC_ERROR_NULL_EXPR;
    }

    return parserEvaluateExpression(expression, result, err_pos);
}
