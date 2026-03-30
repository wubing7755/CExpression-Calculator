/**
 * @file token.h
 * @brief Token 类型兼容性头文件
 * 
 * ## 历史说明
 * 
 * 此头文件曾经是公共 API 的一部分，直接导出 Token 和 Lexer 类型。
 * 现在这些类型已经移到 lexer.h 中，保留此文件仅为了向后兼容。
 * 
 * ## 迁移指南
 * 
 * 旧代码：
 * @code
 *   #include "token.h"
 *   Token myToken;  // 错误：Token 未定义
 * @endcode
 * 
 * 新代码：
 * @code
 *   #include "lexer.h"
 *   Token myToken;  // 正确
 * @endcode
 * 
 * ## 重要提示
 * 
 * 解析器和词法分析器的内部实现现在是私有的。
 * 请只使用 calculator.h 中定义的公共接口。
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "calculator.h"

/**
 * @brief 兼容性提示
 * 
 * Token 类型现在定义在 lexer.h 中。
 * 此头文件仅用于保持旧代码的兼容性。
 */
#if defined(TOKEN_TOKEN_DEFINED)
    /* Token 类型已通过其他途径定义 */
#endif

#endif  /* TOKEN_H */
