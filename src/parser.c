/**
 * @file parser.c
 * @brief 语法分析器实现 - 递归下降解析器
 *
 * ## 文法定义
 *
 *   Expression ::= Term { ('+' | '-') Term }
 *   Term       ::= Unary { ('*' | '/') Unary }
 *   Unary      ::= Primary | ('+' | '-') Unary
 *   Primary    ::= Number | '(' Expression ')'
 *
 * ## 运算符优先级（自下而上递增）
 *
 *   最低优先级  ：加减 (+, -)  ← Expression 层
 *   中等优先级 ：乘除 (*, /)  ← Term 层
 *   最高优先级 ：一元 (+, -)   ← Unary 层
 *   原子       ：数字, 括号    ← Primary 层
 *
 * ## 递归调用链
 *
 *   parseExpression()
 *       └─ parseTerm()
 *           └─ parseUnary()
 *               └─ parsePrimary()  ← 递归终止（叶子节点）
 *
 * ## 设计特点
 *
 *   - 4 层结构，每层 < 30 行
 *   - 文法与代码严格一一对应
 *   - 错误处理统一使用 PARSER_CHECK 宏
 *   - 运算符通过 applyMul/applyDiv/applyAdd/applySub 封装
 *   - 一元运算符使用迭代处理，避免栈溢出
 */

#include "parser.h"

#include <math.h>
#include <stdio.h>

#include "lexer.h"
#include "debug.h"

/* ========================================================================
 * 配置常量
 * ======================================================================== */

#define PARSER_DEFAULT_MAX_RECURSION 256U  /**< 最多允许 255 层括号嵌套 */

/* ========================================================================
 * 解析器状态结构
 * ======================================================================== */

typedef struct {
  Lexer lexer;              /**< 词法分析器 */
  CalcError err;            /**< 当前错误状态 */
  size_t err_pos;           /**< 错误发生位置 */
  unsigned depth;           /**< 当前递归深度 */
  unsigned max_depth;       /**< 最大允许深度 */
  unsigned step_index;      /**< 计算步骤计数器（用于调试输出） */

  /* 调试支持已统一到 debug.c/h */
} Parser;

/* ========================================================================
 * 宏定义
 * ======================================================================== */

/**
 * @brief 错误检查宏 - 所有解析函数必须使用
 *
 * 用法：
 *   double result = parseUnary(parser);
 *   PARSER_CHECK();  // 如果有错误，立即返回 0.0
 *
 * 展开后：
 *   do { if ((parser)->err != CALC_OK) return 0.0; } while(0)
 */
#define PARSER_CHECK()                                                         \
  do {                                                                         \
    if ((parser)->err != CALC_OK)                                              \
      return 0.0;                                                              \
  } while (0)

/* ========================================================================
 * 统一调试宏
 * ======================================================================== */

/**
 * @brief Parser 追踪宏 - 单一宏，按级别输出
 *
 * 级别定义：
 *   DEBUG_LEVEL_ERROR  (1): 错误信息
 *   DEBUG_LEVEL_WARN   (2): 警告信息
 *   DEBUG_LEVEL_INFO   (3): 计算步骤 [步骤N]
 *   DEBUG_LEVEL_DEBUG  (4): 详细上下文
 *   DEBUG_LEVEL_TRACE  (5): 函数调用树
 *
 * 用法示例：
 *   PARSER_TRACE(DEBUG_LEVEL_ERROR, "Division by zero at pos %zu", pos);
 *   PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] 读取数字 %.10g", step, value);
 *   PARSER_TRACE(DEBUG_LEVEL_TRACE, "→ parseExpression() depth=%u", parser->depth);
 */
#define PARSER_TRACE(level, fmt, ...)                                           \
  DEBUG_IF((level), DEBUG_MODULE_PARSER,                                        \
      fprintf(g_debug_output ? g_debug_output : stderr,                          \
          "[PARSER] " fmt, ##__VA_ARGS__))

/* ========================================================================
 * 函数原型声明（前向声明）
 * ======================================================================== */

static double parserParseExpression(Parser *parser);
static double parserParseTerm(Parser *parser);
static double parserParseUnary(Parser *parser);
static double parserParsePrimary(Parser *parser);
static void parserSetError(Parser *parser, CalcError err, size_t err_pos);

/* ========================================================================
 * 运算符求值函数
 *
 * 这些函数执行实际的算术运算，通过 DEBUG_PARSER 输出计算步骤。
 * ======================================================================== */

/**
 * @brief 乘法运算
 */
static double applyMul(double a, double b, Parser *parser) {
  parser->step_index++;
  const double result = a * b;
  PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] %.10g * %.10g = %.10g",
               parser->step_index, a, b, result);
  return result;
}

/**
 * @brief 除法运算
 *
 * 检查除数是否为零，发生错误时设置错误状态。
 */
static double applyDiv(double a, double b, Parser *parser, size_t err_pos) {
  if (fpclassify(b) == FP_ZERO) {
    parserSetError(parser, CALC_ERROR_DIV_BY_ZERO, err_pos);
    PARSER_TRACE(DEBUG_LEVEL_ERROR, "Division by zero at position %zu", err_pos);
    return 0.0;
  }

  parser->step_index++;
  const double result = a / b;
  PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] %.10g / %.10g = %.10g",
               parser->step_index, a, b, result);
  return result;
}

/**
 * @brief 加法运算
 */
static double applyAdd(double a, double b, Parser *parser) {
  parser->step_index++;
  const double result = a + b;
  PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] %.10g + %.10g = %.10g",
               parser->step_index, a, b, result);
  return result;
}

/**
 * @brief 减法运算
 */
static double applySub(double a, double b, Parser *parser) {
  parser->step_index++;
  const double result = a - b;
  PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] %.10g - %.10g = %.10g",
               parser->step_index, a, b, result);
  return result;
}

/* ========================================================================
 * 错误处理
 * ======================================================================== */

/**
 * @brief 设置解析器错误状态
 *
 * 仅设置第一个错误（first error only），避免错误信息被覆盖。
 *
 * @param parser   解析器
 * @param err      错误码
 * @param err_pos  错误位置
 */
static void parserSetError(Parser *parser, CalcError err, size_t err_pos) {
  if (parser->err == CALC_OK) {  /* 仅记录第一个错误 */
    parser->err = err;
    parser->err_pos = err_pos;
  }
}

/**
 * @brief 获取下一个 Token 并传播错误
 *
 * 调用词法分析器的 lexerNextToken，然后检查是否发生错误。
 * 如果词法分析出错，将错误传播到解析器状态。
 *
 * @param parser 解析器
 */
static void parserNextToken(Parser *parser) {
  lexerNextToken(&parser->lexer);

  if (parser->lexer.err != CALC_OK) {
    parserSetError(parser, parser->lexer.err, parser->lexer.err_pos);
  }
}

/* ========================================================================
 * 语法分析核心函数（4层结构）
 * ======================================================================== */

/**
 * @brief 解析 Primary（原子表达式）
 *
 * 文法：Primary ::= Number | '(' Expression ')'
 *
 * 职责：
 *   - 解析数字常量
 *   - 解析括号表达式（消费 '(' → 递归解析 Expression → 消费 ')'）
 *   - 不处理任何运算符
 *
 * @param parser 解析器
 * @return 计算结果
 */
static double parserParsePrimary(Parser *parser) {
  PARSER_TRACE(DEBUG_LEVEL_TRACE, "→ parsePrimary() depth=%u", parser->depth);

  /* 情况 1：数字 */
  if (parser->lexer.current.type == TOKEN_NUMBER) {
    const double value = parser->lexer.current.value;
    parser->step_index++;
    PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] 读取数字 %.10g", parser->step_index, value);
    parserNextToken(parser);
    return value;
  }

  /* 情况 2：左括号 */
  if (parser->lexer.current.type == TOKEN_LPAREN) {
    const size_t lparen_pos = parser->lexer.current.start_pos;

    parserNextToken(parser); /* 消费 '(' */
    PARSER_CHECK();

    /* 递归深度检查 */
    if (parser->depth >= parser->max_depth) {
      parserSetError(parser, CALC_ERROR_RECURSION_LIMIT, lparen_pos);
      return 0.0;
    }

    parser->depth++;
    double value = parserParseExpression(parser); /* 递归解析内部 */
    parser->depth--;
    PARSER_CHECK();

    if (parser->lexer.current.type != TOKEN_RPAREN) {
      parserSetError(parser, CALC_ERROR_MISSING_RPAREN,
                     parser->lexer.current.start_pos);
      return 0.0;
    }

    parserNextToken(parser); /* 消费 ')' */
    return value;
  }

  /* 情况 3：语法错误 */
  parserSetError(parser, CALC_ERROR_UNEXPECTED_TOKEN,
                 parser->lexer.current.start_pos);
  return 0.0;
}

/**
 * @brief 解析 Unary（一元运算）
 *
 * 文法：Unary ::= Primary | ('+' | '-') Unary
 *
 * 职责：
 *   - 识别一元正号/负号（可连续）
 *   - 迭代处理运算符链，避免栈溢出
 *
 * 优化：使用迭代替代右递归，O(1) 空间复杂度
 *
 * @param parser 解析器
 * @return 计算结果
 */
static double parserParseUnary(Parser *parser) {
  PARSER_TRACE(DEBUG_LEVEL_TRACE, "→ parseUnary() depth=%u", parser->depth);

  /* 迭代处理一元运算符链：统计负号个数 */
  int negative_count = 0;
  while (parser->lexer.current.type == TOKEN_PLUS ||
         parser->lexer.current.type == TOKEN_MINUS) {
    if (parser->lexer.current.type == TOKEN_MINUS) {
      negative_count++;
    }
    parserNextToken(parser);
    PARSER_CHECK();
  }

  /* 解析操作数 */
  double result = parserParsePrimary(parser);
  PARSER_CHECK();

  /* 应用负号（奇数个负号则取反） */
  if (negative_count > 0) {
    result = -result;
    PARSER_TRACE(DEBUG_LEVEL_INFO, "[步骤%u] (-)^%d %.10g = %.10g",
                 parser->step_index, negative_count, -result, result);
  }

  return result;
}

/**
 * @brief 解析 Term（乘除运算）
 *
 * 文法：Term ::= Unary { ('*' | '/') Unary }
 *
 * 职责：
 *   - 处理乘除运算（左结合）
 *   - 调用 parseUnary 获取操作数
 *
 * @param parser 解析器
 * @return 计算结果
 */
static double parserParseTerm(Parser *parser) {
  PARSER_TRACE(DEBUG_LEVEL_TRACE, "→ parseTerm() depth=%u", parser->depth);

  double left = parserParseUnary(parser); /* 解析第一个操作数 */
  PARSER_CHECK();

  /* 循环处理连续的乘除运算 */
  while (parser->lexer.current.type == TOKEN_MUL ||
         parser->lexer.current.type == TOKEN_DIV) {

    const TokenType op = parser->lexer.current.type;
    const size_t op_pos = parser->lexer.current.start_pos;

    parserNextToken(parser); /* 消费运算符 */
    PARSER_CHECK();

    double right = parserParseUnary(parser); /* 解析右操作数 */
    PARSER_CHECK();

    /* 执行运算（通过安全函数） */
    if (op == TOKEN_MUL) {
      left = applyMul(left, right, parser);
    } else {
      left = applyDiv(left, right, parser, op_pos);
      PARSER_CHECK(); /* 检查除零错误 */
    }
  }

  return left;
}

/**
 * @brief 解析 Expression（加减运算）
 *
 * 文法：Expression ::= Term { ('+' | '-') Term }
 *
 * 职责：
 *   - 处理加减运算（左结合）
 *   - 调用 parseTerm 获取操作数
 *
 * @param parser 解析器
 * @return 计算结果
 */
static double parserParseExpression(Parser *parser) {
  PARSER_TRACE(DEBUG_LEVEL_TRACE, "→ parseExpression() depth=%u", parser->depth);

  double left = parserParseTerm(parser); /* 解析第一个操作数 */
  PARSER_CHECK();

  /* 循环处理连续的加减运算 */
  while (parser->lexer.current.type == TOKEN_PLUS ||
         parser->lexer.current.type == TOKEN_MINUS) {

    const TokenType op = parser->lexer.current.type;

    parserNextToken(parser); /* 消费运算符 */
    PARSER_CHECK();

    double right = parserParseTerm(parser); /* 解析右操作数 */
    PARSER_CHECK();

    /* 执行运算（使用抽象函数，与 Term 层对称） */
    if (op == TOKEN_PLUS) {
      left = applyAdd(left, right, parser);
    } else {
      left = applySub(left, right, parser);
    }
  }

  return left;
}

/* ========================================================================
 * 公共 API
 * ======================================================================== */

/**
 * @brief 评估数学表达式
 *
 * 解析并计算表达式的值。
 *
 * 处理流程：
 *   1. 初始化词法分析器
 *   2. 设置解析器状态和选项
 *   3. 获取第一个 Token
 *   4. 递归下降解析表达式
 *   5. 检查是否所有 Token 都被消费
 *   6. 返回结果或错误
 *
 * @param expression 要计算的数学表达式
 * @param options    评估选项（可为 NULL）
 * @param result     结果输出
 * @param err_pos    错误位置（可为 NULL）
 * @return 错误码
 */
CalcError parserEvaluateExpression(const char *expression,
                                   double *result, size_t *err_pos) {
  Parser parser;

  /* 初始化词法分析器 */
  lexerInit(&parser.lexer, expression);

#if DEBUG_ENABLE
  /* 预扫描所有 token 用于调试输出 */
  lexerDebugPrintAll(&parser.lexer);
#endif

  /* 初始化解析器状态 */
  parser.err = CALC_OK;
  parser.err_pos = 0;
  parser.depth = 0U;
  parser.step_index = 0U;
  parser.max_depth = PARSER_DEFAULT_MAX_RECURSION;

  /* 读取第一个 token */
  parserNextToken(&parser);

  /* 执行表达式解析 */
  const double value = parserParseExpression(&parser);

  /* 语法检查：确保所有 token 都被消费 */
  if (parser.err == CALC_OK && parser.lexer.current.type != TOKEN_END) {
    parserSetError(&parser, CALC_ERROR_SYNTAX, parser.lexer.current.start_pos);
  }

  /* 返回结果或错误 */
  if (parser.err != CALC_OK) {
    if (err_pos != NULL) {
      *err_pos = parser.err_pos;
    }
    return parser.err;
  }

  *result = value;
  if (err_pos != NULL) {
    *err_pos = 0;
  }
  return CALC_OK;
}