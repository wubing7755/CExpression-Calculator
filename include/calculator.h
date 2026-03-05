/*******************************************************************************
 * calculator.h - 计算器核心逻辑头文件
 * 
 * 本文件声明了表达式求值所需的所有函数
 * 
 * 使用方法：
 *   #include "calculator.h"
 ******************************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>

/**
 * evaluate - 表达式求值主函数
 * 
 * 这是计算器的入口点，接收一个数学表达式字符串，返回计算结果
 * 
 * @param expression 输入的数学表达式字符串（如 "2+3*4"）
 * @param result 用于存储计算结果的指针
 * @return 0 表示成功，非0表示出错
 * 
 * 使用示例：
 *   double result;
 *   evaluate("(3-5)*6", &result);  // result = -12
 */
int evaluate(const char* expression, double* result);

#endif // CALCULATOR_H
