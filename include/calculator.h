/**
 * @file calculator.h
 * @brief 计算器公共接口定义
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>
#include <stddef.h>

/* ========================================================================
 * 错误码枚举
 * ======================================================================== */

typedef enum {
  CALC_OK = 0,                 /**< 成功，无错误 */
  CALC_ERROR_NULL_EXPR,        /**< 表达式为空或 NULL */
  CALC_ERROR_INVALID_CHAR,     /**< 包含非法字符 */
  CALC_ERROR_DIV_BY_ZERO,      /**< 除数为零 */
  CALC_ERROR_UNEXPECTED_TOKEN, /**< 意外的 token */
  CALC_ERROR_MISSING_RPAREN,   /**< 缺少右括号 ) */
  CALC_ERROR_SYNTAX,           /**< 语法错误 */
  CALC_ERROR_RECURSION_LIMIT,  /**< 递归深度超限 */
  CALC_ERROR_NUMBER_OVERFLOW,  /**< 数值超出范围（新增） */
  CALC_ERROR_LAST              /**< 错误码边界标记 */
} CalcError;

/* ========================================================================
 * 步骤回调接口
 * ======================================================================== */

typedef struct {
  unsigned step_index;
  char message[256];   /**< 内嵌缓冲区，避免栈对象生命周期问题 */
  double elapsed_ms;
} CalcStepInfo;

typedef void (*CalcStepCallback)(void *user_data, const CalcStepInfo *step);

/* ========================================================================
 * 评估选项结构
 * ======================================================================== */

typedef struct {
  unsigned max_recursion_depth; /**< 最大递归深度（0=默认256层） */
  bool measure_step_time;       /**< 是否测量每步耗时 */
  CalcStepCallback on_step;     /**< 步骤回调函数（可为NULL） */
  void *user_data;              /**< 用户数据（传递给回调） */
} CalcEvalOptions;

/* ========================================================================
 * 公共函数声明
 * ======================================================================== */

void calcEvalOptionsInit(CalcEvalOptions *options);

CalcError evaluate(const char *expression, const CalcEvalOptions *options,
                   double *result, size_t *err_pos);

const char *calcGetErrorMessage(CalcError err);
const char *parserGetErrorMessage(CalcError err);

#endif /* CALCULATOR_H */