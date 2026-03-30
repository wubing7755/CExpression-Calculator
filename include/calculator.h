#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>
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

typedef struct {
    unsigned step_index;
    const char* message;
    double elapsed_ms;
} CalcStepInfo;

typedef void (*CalcStepCallback)(void* user_data, const CalcStepInfo* step);

typedef struct {
    unsigned max_recursion_depth;
    bool measure_step_time;
    CalcStepCallback on_step;
    void* user_data;
} CalcEvalOptions;

void calcEvalOptionsInit(CalcEvalOptions* options);
CalcError evaluate(const char* expression,
                   const CalcEvalOptions* options,
                   double* result,
                   size_t* err_pos);
const char* calcGetErrorMessage(CalcError err);

/* Backward-compatible alias used by existing code. */
const char* parserGetErrorMessage(CalcError err);

#endif // CALCULATOR_H
