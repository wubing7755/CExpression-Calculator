/**
 * @file calculator.c
 * @brief 计算器公共接口实现
 * 
 * ## 模块职责
 * 
 * 这个文件是计算器功能的公共入口点，负责：
 *   1. 定义错误消息（供外部显示）
 *   2. 提供公共 API（evaluate 函数）
 *   3. 管理评估选项的初始化
 * 
 * ## 与其他模块的关系
 * 
 *   calculator.c (此文件)
 *       │
 *       ├── 定义 CalcError 枚举和 CalcEvalOptions 结构
 *       │
 *       └── parser.c (实际解析逻辑)
 *               │
 *               └── lexer.c (词法分析)
 * 
 * 核心计算逻辑实际上在 parser.c 中实现，这里只是提供一层封装。
 */

#include "calculator.h"

#include "parser.h"

/* ========================================================================
 * 错误消息定义
 * ======================================================================== */

/**
 * @brief 错误消息表
 * 
 * 使用"指定初始化器"（Designated Initializer）将 CalcError 枚举值映射到人类可读的错误消息。
 * 
 * ## 为什么使用这种语法？
 * 
 * 传统的数组初始化要求元素按顺序排列，但这种方式：
 *   - 容易出错（如果枚举值顺序改变）
 *   - 不够清晰（看不出哪个消息对应哪个错误）
 * 
 * 使用指定初始化器 [KEY] = value 的语法：
 *   - 即使枚举顺序改变也能正确匹配
 *   - 代码更自文档化
 *   - 添加新错误类型时不易遗漏
 * 
 * ## 错误类型说明
 * 
 *   CALC_OK                  : 成功，无错误
 *   CALC_ERROR_NULL_EXPR    : 表达式为 NULL 或空字符串
 *   CALC_ERROR_INVALID_CHAR : 输入包含无法识别的字符
 *   CALC_ERROR_DIV_BY_ZERO  : 除数为零
 *   CALC_ERROR_UNEXPECTED_TOKEN : 语法错误，遇到了意外的 token
 *   CALC_ERROR_MISSING_RPAREN : 缺少右括号
 *   CALC_ERROR_SYNTAX       : 通用语法错误
 *   CALC_ERROR_RECURSION_LIMIT : 递归深度超限（括号嵌套太深）
 */
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

/* ========================================================================
 * 编译期安全检查
 * ======================================================================== */

/**
 * @brief 静态断言 - 编译期检查
 * 
 * _Static_assert 是 C11 引入的编译期断言机制。
 * 如果条件为假，编译器会在编译时报错，而不是等到运行时才发现问题。
 * 
 * ## 这里检查什么？
 * 
 * 确保 g_error_messages 数组的大小与 CalcError 枚举的定义完全匹配。
 * 如果有人添加了新的 CalcError 但忘记在数组中添加对应的消息，
 * 编译时会报错：
 *   "Error message array size does not match CalcError enum"
 * 
 * ## sizeof 技巧
 * 
 * sizeof(g_error_messages)       : 整个数组的字节大小
 * sizeof(g_error_messages[0])    : 一个元素的字节大小
 * 相除得到元素个数
 * 
 * 注意：这里的检查假设 CalcError 枚举是连续编号的（从 0 开始），
 * 并且最后一个枚举值 CALC_ERROR_RECURSION_LIMIT 代表了最大的错误码。
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(g_error_messages) / sizeof(g_error_messages[0]) == CALC_ERROR_RECURSION_LIMIT + 1,
               "Error message array size does not match CalcError enum");
#endif

/* ========================================================================
 * 公共 API 实现
 * ======================================================================== */

/**
 * @brief 获取错误消息
 * 
 * 根据错误码返回对应的可读字符串。
 * 如果传入无效的错误码，返回 "Unknown error"。
 * 
 * @param err 错误码
 * @return 错误消息字符串
 */
const char* calcGetErrorMessage(CalcError err)
{
    const size_t count = sizeof(g_error_messages) / sizeof(g_error_messages[0]);
    
    /* 检查错误码是否在有效范围内 */
    if (err >= 0 && (size_t)err < count && g_error_messages[err] != NULL) {
        return g_error_messages[err];
    }
    
    return "Unknown error";
}

/**
 * @brief 获取解析器错误消息（向后兼容别名）
 * 
 * 这是 calcGetErrorMessage() 的别名，保持与旧代码的兼容性。
 * 原来的代码可能调用 parserGetErrorMessage()，
 * 现在统一使用 calcGetErrorMessage()。
 * 
 * @param err 错误码
 * @return 错误消息字符串
 */
const char* parserGetErrorMessage(CalcError err)
{
    return calcGetErrorMessage(err);
}

/**
 * @brief 初始化评估选项结构
 * 
 * 将 CalcEvalOptions 结构设置为安全的默认值。
 * 调用此函数后，回调函数指针为 NULL，递归深度无限（0表示使用默认值）。
 * 
 * ## 使用建议
 * 
 * 在调用 evaluate() 之前，如果你想使用默认设置，
 * 可以调用此函数来确保结构被正确初始化。
 * 
 * 或者，你也可以直接手动设置各个字段。
 * 
 * @param options 要初始化的选项结构指针
 */
void calcEvalOptionsInit(CalcEvalOptions* options)
{
    if (options == NULL) {
        return;
    }
    
    /* 设置默认值 */
    options->max_recursion_depth = 0U;     /* 0 表示使用解析器的默认最大值 */
    options->measure_step_time = false;   /* 默认不测量每步耗时 */
    options->on_step = NULL;              /* 默认不调用步骤回调 */
    options->user_data = NULL;            /* 默认用户数据为 NULL */
}

/**
 * @brief 评估数学表达式
 * 
 * 这是计算器的主要公共接口函数。用户调用此函数来计算表达式的值。
 * 
 * ## 函数行为
 * 
 *   1. 参数验证：
 *      - expression 不能为 NULL 或空字符串
 *      - result 不能为 NULL
 *      - 如果验证失败，立即返回错误码，不继续执行
 * 
 *   2. 调用解析器：
 *      - 将工作委托给 parserEvaluateExpression()
 *      - 解析器会进行词法分析和语法分析
 * 
 *   3. 返回结果：
 *      - 成功：*result 包含计算结果
 *      - 失败：*err_pos 包含错误位置（如果提供了此参数）
 * 
 * ## options 参数说明
 * 
 * 可以为 NULL，表示使用默认选项：
 *   - 最大递归深度：256 层括号嵌套
 *   - 不测量步骤时间
 *   - 不调用步骤回调
 * 
 * 如果提供非 NULL 的 options：
 *   - max_recursion_depth: 自定义最大递归深度
 *   - measure_step_time: 是否测量每步耗时
 *   - on_step: 每一步计算的回调函数
 *   - user_data: 传递给回调的用户数据
 * 
 * ## 使用示例
 * 
 * @code
 *   // 简单用法
 *   double result;
 *   size_t err_pos;
 *   CalcError err = evaluate("2 + 3 * 4", NULL, &result, &err_pos);
 *   if (err == CALC_OK) {
 *       printf("Result: %g\n", result);
 *   }
 * 
 *   // 带步骤回调
 *   CalcEvalOptions opts;
 *   calcEvalOptionsInit(&opts);
 *   opts.on_step = my_callback;
 *   evaluate("2 + 3", &opts, &result, NULL);
 * @endcode
 * 
 * @param expression 要计算的数学表达式字符串
 * @param options 评估选项（可为 NULL 使用默认选项）
 * @param result 输出参数，用于存储计算结果
 * @param err_pos 输出参数，用于存储错误位置（可为 NULL）
 * @return 错误码，CALC_OK 表示成功
 */
CalcError evaluate(const char* expression,
                   const CalcEvalOptions* options,
                   double* result,
                   size_t* err_pos)
{
    /* ---------------------------------------------------------------
     * 参数验证
     * ---------------------------------------------------------------
     * 
     * 防御性编程：在函数开始时验证所有输入参数。
     * 任何 NULL 或无效参数都应该被拒绝，而不是让程序在后续崩溃。
     */
    if (expression == NULL || *expression == '\0' || result == NULL) {
        if (err_pos != NULL) {
            *err_pos = 0;
        }
        return CALC_ERROR_NULL_EXPR;
    }

    /* ---------------------------------------------------------------
     * 委托给解析器
     * ---------------------------------------------------------------
     * 
     * 实际的词法分析和语法分析工作在这里完成。
     * parserEvaluateExpression() 会：
     *   1. 初始化词法分析器
     *   2. 逐个读取 token
     *   3. 使用递归下降算法解析表达式
     *   4. 计算结果或报告错误
     */
    return parserEvaluateExpression(expression, options, result, err_pos);
}
