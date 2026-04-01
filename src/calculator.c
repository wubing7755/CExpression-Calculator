/**
 * @file calculator.c
 * @brief 计算器公共接口实现
 */

#include "calculator.h"

#include "parser.h"

/* ========================================================================
 * 错误消息定义
 * ======================================================================== */

static const char *g_error_messages[] = {
    [CALC_OK] = "Success",
    [CALC_ERROR_NULL_EXPR] = "Expression is NULL or empty",
    [CALC_ERROR_INVALID_CHAR] = "Invalid character found",
    [CALC_ERROR_DIV_BY_ZERO] = "Division by zero",
    [CALC_ERROR_UNEXPECTED_TOKEN] = "Unexpected token",
    [CALC_ERROR_MISSING_RPAREN] = "Missing closing parenthesis",
    [CALC_ERROR_SYNTAX] = "Syntax error",
    [CALC_ERROR_RECURSION_LIMIT] = "Expression nesting is too deep",
    [CALC_ERROR_NUMBER_OVERFLOW] = "Number out of representable range",
    [CALC_ERROR_LAST] = NULL};

/* ========================================================================
 * 编译期安全检查
 * ======================================================================== */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(g_error_messages) / sizeof(g_error_messages[0]) ==
                   CALC_ERROR_LAST,
               "Error message array size does not match CalcError enum");
#endif

/* ========================================================================
 * 公共 API 实现
 * ======================================================================== */

const char *calcGetErrorMessage(CalcError err) {
  const size_t count = sizeof(g_error_messages) / sizeof(g_error_messages[0]);

  if (err >= 0 && (size_t)err < count && g_error_messages[err] != NULL) {
    return g_error_messages[err];
  }

  return "Unknown error";
}

const char *parserGetErrorMessage(CalcError err) {
  return calcGetErrorMessage(err);
}

void calcEvalOptionsInit(CalcEvalOptions *options) {
  if (options == NULL) {
    return;
  }

  options->max_recursion_depth = 0U;
  options->measure_step_time = false;
}

/**
 * @brief 评估数学表达式
 *
 * 公共入口函数，对表达式进行验证后委托给 parser 模块。
 *
 * @param expression 要计算的数学表达式
 * @param options   评估选项（可为 NULL）
 * @param result    结果输出指针
 * @param err_pos   错误位置（可为 NULL）
 * @return 错误码
 */
CalcError evaluate(const char *expression, const CalcEvalOptions *options,
                   double *result, size_t *err_pos) {
  /* 参数验证：表达式为空或结果指针为 NULL */
  if (expression == NULL || *expression == '\0' || result == NULL) {
    if (err_pos != NULL) {
      *err_pos = 0;
    }
    return CALC_ERROR_NULL_EXPR;
  }

  return parserEvaluateExpression(expression, options, result, err_pos);
}