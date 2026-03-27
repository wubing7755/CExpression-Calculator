/*******************************************************************************
 * calculator.h - 计算器核心逻辑头文件
 * 
 * 本文件声明了表达式求值所需的所有函数
 * 
 * 使用方法：
 *   #include "calculator.h"
 * 
 * API使用示例：
 *   double result;
 *   CalcError err = evaluate("2+3*4", &result);
 *   if (err != CALC_OK) {
 *       printf("Error: %s\n", parserGetErrorMessage(err));
 *   }
 ******************************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>
#include "token.h"

/*=============================================================================
 * 公共API
 *===========================================================================*/

/**
 * evaluate - 表达式求值主函数
 * 
 * 接收一个数学表达式字符串，返回计算结果
 * 支持详细的错误码返回，便于调试
 * 
 * @param expression 输入的数学表达式字符串（如 "2+3*4"）
 * @param result 用于存储计算结果的指针
 * @param err_pos 若非 NULL，返回出错位置（0 基于原始字符串索引）
 * @return CalcError 错误码，CALC_OK 表示成功
 * 
 * 使用示例：
 *   double result;
 *   CalcError err = evaluate("(3-5)*6", &result, 0);  // result = -12, err = CALC_OK
 */
CalcError evaluate(const char* expression, double* result, size_t* err_pos);

#endif // CALCULATOR_H
