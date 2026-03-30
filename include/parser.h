#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "calculator.h"

CalcError parserEvaluateExpression(const char* expression,
                                   const CalcEvalOptions* options,
                                   double* result,
                                   size_t* err_pos);

#endif
