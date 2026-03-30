/**
 * @file calculator.h
 * @brief 计算器公共接口定义
 * 
 * ## 模块介绍
 * 
 * 这是计算器功能的核心头文件，定义了：
 *   - 错误码枚举 (CalcError)
 *   - 评估选项结构 (CalcEvalOptions)
 *   - 步骤回调接口 (CalcStepCallback, CalcStepInfo)
 *   - 公共函数声明
 * 
 * ## 架构概览
 * 
 * @code
 * 
 *   用户代码
 *       │
 *       ▼
 *   calculator.h / calculator.c  ← 公共接口
 *       │
 *       ▼
 *   parser.h / parser.c          ← 语法分析（递归下降）
 *       │
 *       ▼
 *   lexer.h / lexer.c            ← 词法分析
 * 
 * @endcode
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>
#include <stddef.h>

/* ========================================================================
 * 错误码枚举
 * ======================================================================== */

/**
 * @brief 计算器错误码
 * 
 * 定义所有可能的错误类型。
 * 
 * ## 错误分类
 * 
 * ### 输入错误
 *   - CALC_ERROR_NULL_EXPR     : 表达式为空或 NULL
 *   - CALC_ERROR_INVALID_CHAR  : 包含无法识别的字符
 * 
 * ### 语法错误
 *   - CALC_ERROR_SYNTAX        : 通用语法错误
 *   - CALC_ERROR_UNEXPECTED_TOKEN: 意外的 token
 *   - CALC_ERROR_MISSING_RPAREN: 缺少右括号
 * 
 * ### 数学错误
 *   - CALC_ERROR_DIV_BY_ZERO   : 除数为零
 * 
 * ### 运行时错误
 *   - CALC_ERROR_RECURSION_LIMIT: 括号嵌套太深
 * 
 * ## 使用建议
 * 
 * 使用 calcGetErrorMessage() 获取人类可读的错误描述。
 */
typedef enum {
    CALC_OK = 0,                       /**< 成功，无错误 */
    CALC_ERROR_NULL_EXPR,              /**< 表达式为空或 NULL */
    CALC_ERROR_INVALID_CHAR,          /**< 包含非法字符 */
    CALC_ERROR_DIV_BY_ZERO,           /**< 除数为零 */
    CALC_ERROR_UNEXPECTED_TOKEN,      /**< 意外的 token */
    CALC_ERROR_MISSING_RPAREN,        /**< 缺少右括号 ) */
    CALC_ERROR_SYNTAX,                /**< 语法错误 */
    CALC_ERROR_RECURSION_LIMIT        /**< 递归深度超限 */
} CalcError;

/* ========================================================================
 * 步骤回调接口
 * ======================================================================== */

/**
 * @brief 计算步骤信息结构
 * 
 * 当启用"显示计算过程"时，每一步都会填充此结构。
 * 
 * ## 字段说明
 * 
 *   - step_index : 步骤序号（从1开始）
 *   - message    : 步骤描述（如 "2 * 3 = 6"）
 *   - elapsed_ms : 此步骤的耗时（毫秒，可选功能）
 */
typedef struct {
    unsigned step_index;      /**< 步骤序号 */
    const char* message;      /**< 步骤描述信息 */
    double elapsed_ms;        /**< 步骤耗时（毫秒） */
} CalcStepInfo;

/**
 * @brief 步骤回调函数类型
 * 
 * 每执行一个计算步骤都会调用此函数。
 * 
 * ## 使用场景
 * 
 *   - 调试：观察表达式如何被解析
 *   - 教学：展示计算过程
 *   - 性能分析：测量每步耗时
 * 
 * @param user_data 用户数据（来自 CalcEvalOptions.user_data）
 * @param step 步骤信息结构
 */
typedef void (*CalcStepCallback)(void* user_data, const CalcStepInfo* step);

/* ========================================================================
 * 评估选项结构
 * ======================================================================== */

/**
 * @brief 表达式评估选项
 * 
 * 用于自定义评估行为。
 * 
 * ## 字段说明
 * 
 *   - max_recursion_depth : 最大递归深度（0=使用默认值256）
 *   - measure_step_time   : 是否测量每步耗时
 *   - on_step             : 步骤回调函数（NULL=不调用）
 *   - user_data           : 传递给回调的用户数据
 * 
 * ## 使用示例
 * 
 * @code
 *   CalcEvalOptions opts = {0};  // C99 初始化为全零
 *   opts.on_step = my_callback;
 *   opts.measure_step_time = true;
 *   
 *   evaluate("2+3", &opts, &result, NULL);
 * @endcode
 */
typedef struct {
    unsigned max_recursion_depth;  /**< 最大递归深度（0=默认256层） */
    bool measure_step_time;       /**< 是否测量每步耗时 */
    CalcStepCallback on_step;      /**< 步骤回调函数（可为NULL） */
    void* user_data;               /**< 用户数据（传递给回调） */
} CalcEvalOptions;

/* ========================================================================
 * 公共函数声明
 * ======================================================================== */

/**
 * @brief 初始化评估选项
 * 
 * 将选项结构设置为安全的默认值。
 * 
 * @param options 要初始化的选项指针
 */
void calcEvalOptionsInit(CalcEvalOptions* options);

/**
 * @brief 评估数学表达式
 * 
 * 计算表达式的值并返回结果。
 * 
 * @param expression 数学表达式字符串
 * @param options 评估选项（可为NULL使用默认）
 * @param result 结果输出
 * @param err_pos 错误位置输出（可为NULL）
 * @return 错误码，CALC_OK表示成功
 */
CalcError evaluate(const char* expression,
                   const CalcEvalOptions* options,
                   double* result,
                   size_t* err_pos);

/**
 * @brief 获取错误消息
 * 
 * 根据错误码返回人类可读的错误描述。
 * 
 * @param err 错误码
 * @return 错误消息字符串
 */
const char* calcGetErrorMessage(CalcError err);

/**
 * @brief 获取解析器错误消息（向后兼容）
 * 
 * @deprecated 请使用 calcGetErrorMessage()
 * 
 * @param err 错误码
 * @return 错误消息字符串
 */
const char* parserGetErrorMessage(CalcError err);

#endif  /* CALCULATOR_H */
