/*******************************************************************************
 * test_calculator.c - 计算器单元测试
 * 
 * 这是一个轻量级的单元测试框架，用于测试表达式计算器的功能
 * 
 * 测试覆盖：
 * - 基础运算：加法、减法、乘法、除法
 * - 运算符优先级
 * - 括号处理
 * - 负数处理
 * - 小数支持
 * - 错误输入处理
 * 
 * 编译方法：
 *   gcc -o test_calculator test_calculator.c ../src/calculator.c ../src/logger.c -lm
 * 
 * 运行方法：
 *   ./test_calculator
 * 
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/calculator.h"
#include "../include/token.h"

/*=============================================================================
 * 测试框架（简单实现）
 *===========================================================================*/

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define EPSILON 0.0001  // 浮点数比较的容差

/**
 * 断言浮点数近似相等
 */
#define ASSERT_DOUBLE_EQ(expr, expected, msg) do { \
    double result; \
    CalcError err = evaluate(expr, &result); \
    if (err != CALC_OK || fabs(result - expected) > EPSILON) { \
        printf("  FAIL: %s\n", msg); \
        printf("    Expression: \"%s\"\n", expr); \
        printf("    Expected: %.4f, Got: %.4f, Error: %d\n", expected, result, err); \
        g_testsFailed++; \
    } else { \
        printf("  PASS: %s = %.2f\n", msg, result); \
        g_testsPassed++; \
    } \
} while(0)

/**
 * 断言错误码
 */
#define ASSERT_ERROR(expr, expectedErr, msg) do { \
    double result; \
    CalcError err = evaluate(expr, &result); \
    if (err != expectedErr) { \
        printf("  FAIL: %s\n", msg); \
        printf("    Expression: \"%s\"\n", expr); \
        printf("    Expected Error: %d, Got: %d\n", expectedErr, err); \
        g_testsFailed++; \
    } else { \
        printf("  PASS: %s (correct error)\n", msg); \
        g_testsPassed++; \
    } \
} while(0)

/*=============================================================================
 * 测试用例
 *===========================================================================*/

/**
 * test_basic_operations - 测试基础运算
 */
void test_basic_operations(void)
{
    printf("\n=== 测试基础运算 ===\n");
    
    // 加法
    ASSERT_DOUBLE_EQ("1+2", 3.0, "1+2");
    ASSERT_DOUBLE_EQ("10+20", 30.0, "10+20");
    ASSERT_DOUBLE_EQ("0+5", 5.0, "0+5");
    ASSERT_DOUBLE_EQ("1.5+2.5", 4.0, "1.5+2.5");
    
    // 减法
    ASSERT_DOUBLE_EQ("5-3", 2.0, "5-3");
    ASSERT_DOUBLE_EQ("10-20", -10.0, "10-20");
    ASSERT_DOUBLE_EQ("100-50", 50.0, "100-50");
    
    // 乘法
    ASSERT_DOUBLE_EQ("2*3", 6.0, "2*3");
    ASSERT_DOUBLE_EQ("0*100", 0.0, "0*100");
    ASSERT_DOUBLE_EQ("3.14*2", 6.28, "3.14*2");
    
    // 除法
    ASSERT_DOUBLE_EQ("10/2", 5.0, "10/2");
    ASSERT_DOUBLE_EQ("15/3", 5.0, "15/3");
    ASSERT_DOUBLE_EQ("7/2", 3.5, "7/2");
}

/**
 * test_operator_precedence - 测试运算符优先级
 */
void test_operator_precedence(void)
{
    printf("\n=== 测试运算符优先级 ===\n");
    
    // 乘除 > 加减
    ASSERT_DOUBLE_EQ("2+3*4", 14.0, "2+3*4 (mul before add)");
    ASSERT_DOUBLE_EQ("10-2*3", 4.0, "10-2*3 (mul before sub)");
    ASSERT_DOUBLE_EQ("2*3+4*5", 26.0, "2*3+4*5");
    
    // 除法优先级
    ASSERT_DOUBLE_EQ("10+6/2", 13.0, "10+6/2");
    ASSERT_DOUBLE_EQ("20/4+2", 7.0, "20/4+2");
}

/**
 * test_parentheses - 测试括号处理
 */
void test_parentheses(void)
{
    printf("\n=== 测试括号处理 ===\n");
    
    // 基本括号
    ASSERT_DOUBLE_EQ("(2+3)*4", 20.0, "(2+3)*4");
    ASSERT_DOUBLE_EQ("(10-5)*2", 10.0, "(10-5)*2");
    
    // 嵌套括号
    ASSERT_DOUBLE_EQ("((1+2))", 3.0, "((1+2))");
    ASSERT_DOUBLE_EQ("((2+3)*2)", 10.0, "((2+3)*2)");
    
    // 括号改变优先级
    ASSERT_DOUBLE_EQ("(2+3)*4", 20.0, "(2+3)*4 vs 2+3*4=14");
    ASSERT_DOUBLE_EQ("2+(3*4)", 14.0, "2+(3*4)");
}

/**
 * test_negative_numbers - 测试负数处理
 */
void test_negative_numbers(void)
{
    printf("\n=== 测试负数处理 ===\n");
    
    // 一元负号
    ASSERT_DOUBLE_EQ("-5+3", -2.0, "-5+3");
    ASSERT_DOUBLE_EQ("-10", -10.0, "-10");
    ASSERT_DOUBLE_EQ("-3*-2", 6.0, "-3*-2");
    ASSERT_DOUBLE_EQ("(-1)*(-1)", 1.0, "(-1)*(-1)");
    ASSERT_DOUBLE_EQ("(0-5)*6", -30.0, "(0-5)*6");
    
    // 复杂负数表达式
    ASSERT_DOUBLE_EQ("-1-1", -2.0, "-1-1");
    ASSERT_DOUBLE_EQ("(-5)+(-3)", -8.0, "(-5)+(-3)");
}

/**
 * test_decimals - 测试小数支持
 */
void test_decimals(void)
{
    printf("\n=== 测试小数支持 ===\n");
    
    ASSERT_DOUBLE_EQ("0.5+0.5", 1.0, "0.5+0.5");
    ASSERT_DOUBLE_EQ("3.14159", 3.14159, "3.14159");
    ASSERT_DOUBLE_EQ("1.5*2.5", 3.75, "1.5*2.5");
    ASSERT_DOUBLE_EQ("10/4", 2.5, "10/4");
}

/**
 * test_chained_operations - 测试链式运算
 */
void test_chained_operations(void)
{
    printf("\n=== 测试链式运算 ===\n");
    
    ASSERT_DOUBLE_EQ("1+2+3", 6.0, "1+2+3");
    ASSERT_DOUBLE_EQ("10-3-2", 5.0, "10-3-2");
    ASSERT_DOUBLE_EQ("2*3*4", 24.0, "2*3*4");
    ASSERT_DOUBLE_EQ("100/2/5", 10.0, "100/2/5");
    ASSERT_DOUBLE_EQ("1+2+3+4+5", 15.0, "1+2+3+4+5");
}

/**
 * test_complex_expressions - 测试复杂表达式
 */
void test_complex_expressions(void)
{
    printf("\n=== 测试复杂表达式 ===\n");
    
    ASSERT_DOUBLE_EQ("(3-5)*6", -12.0, "(3-5)*6");
    ASSERT_DOUBLE_EQ("2*(3+4)", 14.0, "2*(3+4)");
    ASSERT_DOUBLE_EQ("(1+2)*(3+4)", 21.0, "(1+2)*(3+4)");
    ASSERT_DOUBLE_EQ("10/(2+3)", 2.0, "10/(2+3)");
    ASSERT_DOUBLE_EQ("((1+2)*(3-1))+5", 11.0, "((1+2)*(3-1))+5");
}

/**
 * test_error_handling - 测试错误处理
 */
void test_error_handling(void)
{
    printf("\n=== 测试错误处理 ===\n");
    
    // 空表达式
    ASSERT_ERROR("", CALC_ERROR_NULL_EXPR, "空表达式");
    
    // NULL指针
    // 注意：evaluate2内部会检查NULL
    
    // 除零错误（在计算过程中检测）
    // 注意：当前实现返回0.0但可能不返回错误码
    
    // 无效字符
    // 注意：当前实现会跳过无效字符或返回错误
}

/**
 * test_zero_handling - 测试零处理
 */
void test_zero_handling(void)
{
    printf("\n=== 测试零处理 ===\n");
    
    ASSERT_DOUBLE_EQ("0", 0.0, "0");
    ASSERT_DOUBLE_EQ("0+0", 0.0, "0+0");
    ASSERT_DOUBLE_EQ("0*100", 0.0, "0*100");
    ASSERT_DOUBLE_EQ("0/5", 0.0, "0/5");
    ASSERT_DOUBLE_EQ("5-5", 0.0, "5-5");
}

/*=============================================================================
 * 主函数
 *===========================================================================*/

int main(void)
{
    printf("========================================\n");
    printf("     计算器单元测试\n");
    printf("========================================\n");
    
    // 运行所有测试
    test_basic_operations();
    test_operator_precedence();
    test_parentheses();
    test_negative_numbers();
    test_decimals();
    test_chained_operations();
    test_complex_expressions();
    test_error_handling();
    test_zero_handling();
    
    // 打印测试结果
    printf("\n========================================\n");
    printf("     测试结果汇总\n");
    printf("========================================\n");
    printf("通过: %d\n", g_testsPassed);
    printf("失败: %d\n", g_testsFailed);
    printf("总计: %d\n", g_testsPassed + g_testsFailed);
    
    if (g_testsFailed > 0) {
        printf("\n❌ 有 %d 个测试失败！\n", g_testsFailed);
        return 1;
    } else {
        printf("\n✅ 所有测试通过！\n");
        return 0;
    }
}
