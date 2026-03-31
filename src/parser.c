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
 * ## 特点
 *
 *   - 4 层结构，每层 < 30 行
 *   - 文法与代码严格一一对应
 *   - 错误处理统一使用 PARSER_CHECK 宏
 *   - 运算符通过 evalMul/evalDiv 封装
 */

#include "parser.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "lexer.h"
#include "debug.h"

/* ========================================================================
 * 配置常量
 * ======================================================================== */

#define PARSER_DEFAULT_MAX_RECURSION 256U

/* ========================================================================
 * 解析器状态结构
 * ======================================================================== */

typedef struct {
  Lexer lexer;              /**< 词法分析器 */
  CalcError err;            /**< 当前错误状态 */
  size_t err_pos;           /**< 错误发生位置 */
  unsigned depth;           /**< 当前递归深度 */
  unsigned max_depth;       /**< 最大允许深度 */
  unsigned step_index;      /**< 计算步骤计数器 */
  bool measure_step_time;   /**< 是否测量步骤耗时 */
  CalcStepCallback on_step; /**< 步骤回调函数 */
  void *step_user_data;     /**< 用户数据 */
  clock_t last_step_clock;  /**< 上一步的时间戳 */

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
 * 调试宏（统一使用 debug.c/h 系统）
 * ======================================================================== */

/**
 * @brief Parser 函数入口跟踪（带缩进的树结构）
 */
#define PARSER_DEBUG_ENTER(func)                                                \
  DEBUG_IF(DEBUG_LEVEL_TRACE, DEBUG_MODULE_PARSER,                              \
      fprintf(g_debug_output ? g_debug_output : stderr,                          \
          "[PARSER] %*s%-4s %s()\n", parser->depth * 4, "", "│  ", func))

/**
 * @brief Parser 函数出口跟踪（仅标记，用于跟踪流）
 */
#define PARSER_DEBUG_EXIT(result)                                               \
  DEBUG_TRACE_EXIT(DEBUG_MODULE_PARSER, (result))

/**
 * @brief Parser 错误调试
 */
#define PARSER_DEBUG_ERROR(fmt, ...)                                            \
  DEBUG_ERROR("[PARSER] " fmt, ##__VA_ARGS__)

/* ========================================================================
 * 函数原型声明（前向声明）
 * ======================================================================== */

static double parserParseExpression(Parser *parser);
static double parserParseTerm(Parser *parser);
static double parserParseUnary(Parser *parser);
static double parserParsePrimary(Parser *parser);
static void parserEmitStep(Parser *parser, const char *fmt, ...);
static void parserSetError(Parser *parser, CalcError err, size_t err_pos);

/* ========================================================================
 * 运算符求值函数
 * ======================================================================== */

/**
 * @brief 乘法运算
 *
 * @param a 左操作数
 * @param b 右操作数
 * @param parser 解析器（用于步骤回调）
 * @return 运算结果
 */
static double evalMul(double a, double b, Parser *parser) {
  const double result = a * b;
  parser->step_index++;
  parserEmitStep(parser, "%.10g * %.10g = %.10g", a, b, result);
  DEBUG_PARSER("%*s%.10g * %.10g = %.10g", parser->depth * 4, "", a, b, result);
  return result;
}

static double evalDiv(double a, double b, Parser *parser, size_t err_pos) {
  if (fpclassify(b) == FP_ZERO) {
    parserSetError(parser, CALC_ERROR_DIV_BY_ZERO, err_pos);
    PARSER_DEBUG_ERROR("Division by zero at position %zu", err_pos);
    return 0.0;
  }

  const double result = a / b;
  parser->step_index++;
  parserEmitStep(parser, "%.10g / %.10g = %.10g", a, b, result);
  DEBUG_PARSER("%*s%.10g / %.10g = %.10g", parser->depth * 4, "", a, b, result);
  return result;
}

static double evalAdd(double a, double b, Parser *parser) {
  const double result = a + b;
  parser->step_index++;
  parserEmitStep(parser, "%.10g + %.10g = %.10g", a, b, result);
  DEBUG_PARSER("%*s%.10g + %.10g = %.10g", parser->depth * 4, "", a, b, result);
  return result;
}

static double evalSub(double a, double b, Parser *parser) {
  const double result = a - b;
  parser->step_index++;
  parserEmitStep(parser, "%.10g - %.10g = %.10g", a, b, result);
  DEBUG_PARSER("%*s%.10g - %.10g = %.10g", parser->depth * 4, "", a, b, result);
  return result;
}

/* ========================================================================
 * 错误处理
 * ======================================================================== */

static void parserSetError(Parser *parser, CalcError err, size_t err_pos) {
  if (parser->err == CALC_OK) {
    parser->err = err;
    parser->err_pos = err_pos;
  }
}

static void parserNextToken(Parser *parser) {
  lexerNextToken(&parser->lexer);

  if (parser->lexer.err != CALC_OK) {
    parserSetError(parser, parser->lexer.err, parser->lexer.err_pos);
  }
}

/* ========================================================================
 * 步骤回调
 * ======================================================================== */

static void parserEmitStep(Parser *parser, const char *fmt, ...) {
  char message[256];
  CalcStepInfo step;
  va_list args;

  if (parser->on_step == NULL) {
    return;
  }

  va_start(args, fmt);
  vsnprintf(message, sizeof(message), fmt, args);
  va_end(args);

  step.step_index = parser->step_index;
  step.message = message;
  step.elapsed_ms = 0.0;

  if (parser->measure_step_time) {
    const clock_t now = clock();
    if (now != (clock_t)-1 && parser->last_step_clock != (clock_t)-1) {
      step.elapsed_ms = (double)(now - parser->last_step_clock) * 1000.0 /
                        (double)CLOCKS_PER_SEC;
    }
    parser->last_step_clock = now;
  }

  parser->on_step(parser->step_user_data, &step);
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
  PARSER_DEBUG_ENTER("parsePrimary");

  /* 情况 1：数字 */
  if (parser->lexer.current.type == TOKEN_NUMBER) {
    const double value = parser->lexer.current.value;

    parser->step_index++;
    parserEmitStep(parser, "读取数字 %.10g", value);

    parserNextToken(parser);
    PARSER_DEBUG_EXIT(value);
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
      PARSER_DEBUG_EXIT(0.0);
      return 0.0;
    }

    parser->depth++;
    double value = parserParseExpression(parser); /* 递归解析内部 */
    parser->depth--;
    PARSER_CHECK();

    if (parser->lexer.current.type != TOKEN_RPAREN) {
      parserSetError(parser, CALC_ERROR_MISSING_RPAREN,
                     parser->lexer.current.start_pos);
      PARSER_DEBUG_EXIT(0.0);
      return 0.0;
    }

    parserNextToken(parser); /* 消费 ')' */
    PARSER_DEBUG_EXIT(value);
    return value;
  }

  /* 情况 3：语法错误 */
  parserSetError(parser, CALC_ERROR_UNEXPECTED_TOKEN,
                 parser->lexer.current.start_pos);
  PARSER_DEBUG_EXIT(0.0);
  return 0.0;
}

/**
 * @brief 解析 Unary（一元运算）
 *
 * 文法：Unary ::= Primary | ('+' | '-') Unary
 *
 * 职责：
 *   - 识别一元正号/负号（可连续）
 *   - 递归调用 parsePrimary 获取操作数
 *
 * 递归终止条件：遇到 Primary（数字或左括号）
 *
 * @param parser 解析器
 * @return 计算结果
 */
static double parserParseUnary(Parser *parser) {
  PARSER_DEBUG_ENTER("parseUnary");

  /* 检查是否有一元运算符（递归情况） */
  if (parser->lexer.current.type == TOKEN_PLUS ||
      parser->lexer.current.type == TOKEN_MINUS) {

    /* 递归深度检查（防止 "----------------1" 这类表达式导致栈溢出） */
    if (parser->depth >= parser->max_depth) {
      parserSetError(parser, CALC_ERROR_RECURSION_LIMIT,
                     parser->lexer.current.start_pos);
      PARSER_DEBUG_EXIT(0.0);
      return 0.0;
    }

    const TokenType op = parser->lexer.current.type;
    parserNextToken(parser); /* 消费运算符 */
    PARSER_CHECK();

    parser->depth++;
    double operand = parserParseUnary(parser); /* 右递归 */
    parser->depth--;
    PARSER_CHECK();

    double result = (op == TOKEN_MINUS) ? -operand : operand;

    parser->step_index++;
    parserEmitStep(parser, "%c%.10g = %.10g", op == TOKEN_MINUS ? '-' : '+',
                   operand, result);

    PARSER_DEBUG_EXIT(result);
    return result;
  }

  /* 递归终止：调用下一层 */
  double result = parserParsePrimary(parser);
  PARSER_DEBUG_EXIT(result);
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
  PARSER_DEBUG_ENTER("parseTerm");

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
      left = evalMul(left, right, parser);
    } else {
      left = evalDiv(left, right, parser, op_pos);
      PARSER_CHECK(); /* 检查除零错误 */
    }
  }

  PARSER_DEBUG_EXIT(left);
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
  PARSER_DEBUG_ENTER("parseExpression");

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
      left = evalAdd(left, right, parser);
    } else {
      left = evalSub(left, right, parser);
    }
  }

  PARSER_DEBUG_EXIT(left);
  return left;
}

/* ========================================================================
 * 公共 API
 * ======================================================================== */

CalcError parserEvaluateExpression(const char *expression,
                                   const CalcEvalOptions *options,
                                   double *result, size_t *err_pos) {
  Parser parser;

  /* 初始化词法分析器 */
  lexerInit(&parser.lexer, expression);

  /* 预扫描所有 token 用于调试输出 */
  lexerDebugPrintAll(&parser.lexer);

  /* 初始化解析器状态 */
  parser.err = CALC_OK;
  parser.err_pos = 0;
  parser.depth = 0U;
  parser.step_index = 0U;
  parser.max_depth = PARSER_DEFAULT_MAX_RECURSION;
  parser.measure_step_time = false;
  parser.on_step = NULL;
  parser.step_user_data = NULL;
  parser.last_step_clock = (clock_t)-1;

  /* 应用用户提供的选项（如果有） */
  if (options != NULL) {
    if (options->max_recursion_depth != 0U) {
      parser.max_depth = options->max_recursion_depth;
    }
    parser.measure_step_time = options->measure_step_time;
    parser.on_step = options->on_step;
    parser.step_user_data = options->user_data;
  }

  if (parser.measure_step_time) {
    parser.last_step_clock = clock();
  }

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