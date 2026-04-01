/**
 * @file parser.h
 * @brief 语法分析器公共接口
 * 
 * ## 模块介绍
 * 
 * 语法分析器（Parser）使用递归下降算法解析数学表达式。
 * 它依赖词法分析器（lexer）提供 token 流。
 * 
 * ## 表达式文法
 * 
 * 本解析器支持的文法规则（BNF 范式）：
 * 
 * @code
 *   Expression ::= Term { ('+' | '-') Term }
 *   Term       ::= Unary { ('*' | '/') Unary }
 *   Unary      ::= Primary | ('+' | '-') Unary
 *   Primary    ::= Number | '(' Expression ')'
 * @endcode
 * 
 * ## 运算符优先级
 * 
 * 文法结构天然地实现了运算符优先级：
 * 
 *   最高优先级：括号 ()，一元负号
 *               ↓
 *   高优先级  ：乘法 *，除法 /
 *               ↓
 *   低优先级  ：加法 +，减法 -
 * 
 * ## 使用示例
 * 
 * @code
 *   double result;
 *   size_t err_pos;
 *   
 *   CalcError err = parserEvaluateExpression(
 *       "2 + 3 * 4",      // 表达式
 *       &result,          // 结果输出
 *       &err_pos          // 错误位置（可选）
 *   );
 *   
 *   if (err == CALC_OK) {
 *       printf("Result: %g\n", result);  // 输出: 14
 *   }
 * @endcode
 */

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "calculator.h"

/**
 * @brief 评估数学表达式
 * 
 * 解析并计算表达式的值。
 * 
 * ## 参数说明
 * 
 * @param expression 要计算的数学表达式字符串
 * @param result 计算结果输出
 * @param err_pos 错误位置输出（可为 NULL）
 * 
 * ## 返回值
 * 
 *   - CALC_OK                    : 成功
 *   - CALC_ERROR_NULL_EXPR      : 表达式为空
 *   - CALC_ERROR_INVALID_CHAR    : 包含非法字符
 *   - CALC_ERROR_DIV_BY_ZERO     : 除数为零
 *   - CALC_ERROR_UNEXPECTED_TOKEN: 语法错误
 *   - CALC_ERROR_MISSING_RPAREN  : 缺少右括号
 *   - CALC_ERROR_SYNTAX          : 通用语法错误
 *   - CALC_ERROR_RECURSION_LIMIT : 递归深度超限
 * 
 * ## 支持的语法
 * 
 *   - 基本运算: +, -, *, /
 *   - 括号   : ()
 *   - 一元负号: -5, --3
 *   - 数字   : 整数、小数 (3.14)、科学计数法 (1e-3)
 * 
 * ## 注意事项
 * 
 *   - 不支持隐式乘法 (2(3) = 2*(3))
 *   - 不支持指数运算 (需使用 pow)
 *   - 不支持变量和函数
 */
CalcError parserEvaluateExpression(const char* expression,
                                   double* result,
                                   size_t* err_pos);

#endif  /* PARSER_H */
