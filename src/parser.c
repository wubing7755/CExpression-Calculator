/**
 * @file parser.c
 * @brief 语法分析器实现 - 递归下降解析器
 * 
 * ## 什么是语法分析？
 * 语法分析（Syntax Analysis/Parsing）是编译原理中的第二步。
 * 在词法分析之后，我们得到了一串 token 流（如 [数字:2, 加号, 数字:3, 乘号, 数字:4]）。
 * 语法分析器的任务是理解这些 token 之间的结构关系，并计算表达式的值。
 * 
 * ## 递归下降解析器（Recursive Descent Parser）
 * 
 * 这是一种自顶向下的解析方法，特别适合处理具有层次结构的表达式。
 * 
 * ### 核心思想
 * 数学表达式可以用一种称为"上下文无关文法"（Context-Free Grammar）来描述：
 * 
 *   表达式 (Expression)  ::= 项 (Term) { 加减运算符 项 }
 *   项     (Term)     ::= 因子 (Factor) { 乘除运算符 因子 }
 *   因子   (Factor)   ::= 数字 | '(' 表达式 ')' | 一元运算符 因子
 * 
 * ### 为什么这样设计？
 * 这种设计巧妙地实现了**运算符优先级**：
 * 
 *   - 乘除 (*) (/) 的优先级高于加减 (+)(-)
 *   - 括号 () 可以改变默认优先级
 * 
 * ### 递归调用链
 *   - parseExpression() 调用 parseTerm()
 *   - parseTerm() 调用 parseFactor()
 *   - parseFactor() 递归调用 parseExpression()（在处理括号时）
 * 
 * ### 解析过程示例
 * 对于表达式 "2 + 3 * 4"：
 * 
 *   parseExpression()
 *   ├─ parseTerm()        // 解析 "2"
 *   │   └─ parseFactor()  // 返回 2
 *   │   // 遇到 '+'，保存当前值 2
 *   │
 *   ├─ 消费 '+'
 *   │
 *   ├─ parseTerm()        // 解析 "3 * 4"
 *   │   ├─ parseFactor() // 返回 3
 *   │   │   // 遇到 '*'
 *   │   ├─ 消费 '*'
 *   │   └─ parseFactor() // 返回 4
 *   │   // 计算 3 * 4 = 12
 *   │   // 返回 12
 *   │
 *   └─ 计算 2 + 12 = 14  // 最终结果
 */

#include "parser.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "lexer.h"

/* ========================================================================
 * 配置常量
 * ======================================================================== */

/**
 * 默认最大递归深度
 * 
 * 用于防止用户输入类似 "((((...))))" 这样层数极深的括号表达式，
 * 导致栈溢出。这个值对于正常使用已经足够（256层嵌套）。
 */
#define PARSER_DEFAULT_MAX_RECURSION 256U

/* ========================================================================
 * 解析器状态结构
 * ======================================================================== */

/**
 * @brief 解析器内部状态
 * 
 * 包含词法分析器实例、错误信息、递归深度追踪和步骤回调。
 * 
 * ## 字段说明
 *   - lexer        : 内嵌的词法分析器
 *   - err          : 当前错误状态
 *   - err_pos      : 错误发生的位置
 *   - depth        : 当前嵌套深度（用于检测括号层数）
 *   - max_depth    : 允许的最大嵌套深度
 *   - step_index   : 计算步骤计数器（用于显示解析过程）
 *   - measure_step_time : 是否测量每步耗时
 *   - on_step      : 步骤回调函数指针
 *   - step_user_data    : 传递给回调的用户数据
 *   - last_step_clock   : 上一步的时间戳
 */
typedef struct {
    Lexer lexer;
    CalcError err;
    size_t err_pos;
    unsigned depth;
    unsigned max_depth;
    unsigned step_index;
    bool measure_step_time;
    CalcStepCallback on_step;
    void* step_user_data;
    clock_t last_step_clock;
} Parser;

/* ========================================================================
 * 函数原型声明（按调用顺序排列）
 * ======================================================================== */

/**
 * @brief 解析表达式入口（供外部调用）
 */
static double parserParseExpression(Parser* parser);

/* ========================================================================
 * 错误处理
 * ======================================================================== */

/**
 * @brief 设置解析器错误状态
 * 
 * 与词法分析器的错误处理类似，只记录第一个错误。
 * 
 * @param parser 解析器实例
 * @param err 错误类型
 * @param err_pos 错误位置
 */
static void parserSetError(Parser* parser, CalcError err, size_t err_pos)
{
    /* 只在无错误时设置，避免覆盖第一次错误 */
    if (parser->err == CALC_OK) {
        parser->err = err;
        parser->err_pos = err_pos;
    }
}

/* ========================================================================
 * Token 前进
 * ======================================================================== */

/**
 * @brief 获取下一个 Token
 * 
 * 对词法分析器的简单封装，同时检查词法错误。
 * 如果词法分析出错，将错误传播到解析器。
 * 
 * @param parser 解析器实例
 */
static void parserNextToken(Parser* parser)
{
    /* 获取下一个 token */
    lexerNextToken(&parser->lexer);
    
    /* 检查词法分析是否出错 */
    if (parser->lexer.err != CALC_OK) {
        parserSetError(parser, parser->lexer.err, parser->lexer.err_pos);
    }
}

/* ========================================================================
 * 计算步骤回调
 * ======================================================================== */

/**
 * @brief 发出计算步骤信息
 * 
 * 当启用"显示计算过程"时，每执行一步都会调用此函数。
 * 它会调用用户提供的回调函数（如果有的话）。
 * 
 * @param parser 解析器实例
 * @param fmt 格式化字符串（类似 printf）
 */
static void parserEmitStep(Parser* parser, const char* fmt, ...)
{
    char message[256];
    CalcStepInfo step;
    va_list args;

    /* 如果没有设置回调函数，直接返回 */
    if (parser->on_step == NULL) {
        return;
    }

    /* 使用 va_list 处理可变参数（类似 vprintf） */
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    /* 填充步骤信息结构 */
    step.step_index = parser->step_index;
    step.message = message;
    step.elapsed_ms = 0.0;

    /* 如果启用了时间测量，计算距上一步的耗时 */
    if (parser->measure_step_time) {
        const clock_t now = clock();
        /* clock() 返回处理器时间，-1 表示不可用 */
        if (now != (clock_t)-1 && parser->last_step_clock != (clock_t)-1) {
            /* 将时钟滴答转换为毫秒 */
            step.elapsed_ms = (double)(now - parser->last_step_clock) * 1000.0 /
                              (double)CLOCKS_PER_SEC;
        }
        parser->last_step_clock = now;
    }

    /* 调用用户的回调函数 */
    parser->on_step(parser->step_user_data, &step);
}

/* ========================================================================
 * 语法分析核心函数
 * ======================================================================== */

/**
 * @brief 解析因子（Factor）
 * 
 * 因子是表达式中最基本的单元，可以是：
 *   1. 一个数字（如 3.14）
 *   2. 带一元运算符的数字（如 -5 或 +3）
 *   3. 括号表达式（如 (2+3)）
 *   4. 带一元运算符的括号表达式（如 -(2+3)）
 * 
 * ## 解析算法
 * 
 * ### 一元运算符处理
 *   - 循环读取 '+' 或 '-' 前缀
 *   - 每个 '-' 使符号翻转一次
 *   - 例如: --5 = +5, ---5 = -5
 * 
 * ### 括号表达式处理
 *   - 遇到 '(' 时，递归调用 parseExpression() 解析内部表达式
 *   - 解析完成后检查是否有对应的 ')'
 *   - 如果没有匹配的右括号，返回错误
 * 
 * ### 递归深度限制
 *   - 每次进入括号表达式，depth++
 *   - 如果 depth > max_depth，说明嵌套太深
 *   - 这是防止栈溢出的安全措施
 * 
 * @param parser 解析器实例
 * @return 计算结果
 */
static double parserParseFactor(Parser* parser)
{
    int sign = 1;  /* 符号位，1 表示正，-1 表示负 */

    /* 如果之前有错误，提前返回 */
    if (parser->err != CALC_OK) {
        return 0.0;
    }

    /* ---------------------------------------------------------------
     * 处理一元运算符（正号和负号）
     * ---------------------------------------------------------------
     * 
     * 循环读取连续的一元运算符。
     * 注意：这里的运算符是"一元"的，作用于紧跟其后的数字或括号。
     * 与后面 parseExpression 中的二元加减运算符不同。
     * 
     * 例如: ---3 被解析为 -(-(-3)) = -3
     */
    while (parser->lexer.current.type == TOKEN_PLUS ||
           parser->lexer.current.type == TOKEN_MINUS) {
        const TokenType unary = parser->lexer.current.type;
        
        /* 如果是负号，翻转符号 */
        if (unary == TOKEN_MINUS) {
            sign = -sign;
        }
        
        /* 获取下一个 token */
        parserNextToken(parser);
        
        /* 检查错误 */
        if (parser->err != CALC_OK) {
            return 0.0;
        }
        
        /* 增加步骤计数并发出步骤信息 */
        parser->step_index++;
        parserEmitStep(parser, "读取一元运算符 '%c'", unary == TOKEN_MINUS ? '-' : '+');
    }

    /* ---------------------------------------------------------------
     * 情况1: 当前 token 是数字
     * --------------------------------------------------------------- */
    if (parser->lexer.current.type == TOKEN_NUMBER) {
        const double raw = parser->lexer.current.value;  /* 原始值 */
        const double value = sign * raw;                  /* 应用符号 */
        
        parser->step_index++;
        parserEmitStep(parser, "读取数字 %.10g%s，结果 %.10g",
                       raw, (sign < 0) ? " 并应用一元负号" : "", value);
        
        /* 重要：读取数字后要调用 nextToken() 前进到下一个 token */
        parserNextToken(parser);
        return value;
    }

    /* ---------------------------------------------------------------
     * 情况2: 当前 token 是左括号
     * ---------------------------------------------------------------
     * 
     * 括号表达式的处理流程：
     *   1. 记录左括号位置（用于错误报告）
     *   2. 消费 '('，读取下一个 token
     *   3. 递归解析括号内的表达式
     *   4. 检查是否有匹配的 ')'
     *   5. 消费 ')'，读取下一个 token
     *   6. 应用外层的一元符号
     */
    if (parser->lexer.current.type == TOKEN_LPAREN) {
        const size_t lparen_pos = parser->lexer.current.start_pos; /* 记录位置 */
        parserNextToken(parser);  /* 消费 '(' */

        /* 检查递归深度，防止栈溢出 */
        if (parser->depth >= parser->max_depth) {
            parserSetError(parser, CALC_ERROR_RECURSION_LIMIT, lparen_pos);
            return 0.0;
        }

        /* 进入括号，深度+1 */
        parser->depth++;
        {
            /* 递归解析括号内的表达式 */
            const double inner = parserParseExpression(parser);
            parser->depth--;  /* 退出括号，深度-1 */

            /* 检查解析是否出错 */
            if (parser->err != CALC_OK) {
                return 0.0;
            }

            /* 检查是否有匹配的右括号 */
            if (parser->lexer.current.type != TOKEN_RPAREN) {
                parserSetError(parser, CALC_ERROR_MISSING_RPAREN,
                               parser->lexer.current.start_pos);
                return 0.0;
            }

            /* 消费 ')'，前进到下一个 token */
            parserNextToken(parser);
            
            /* 应用一元符号（如果有） */
            if (sign < 0) {
                parser->step_index++;
                parserEmitStep(parser, "对括号表达式应用一元负号，结果 %.10g", -inner);
                return -inner;
            }
            return inner;
        }
    }

    /* ---------------------------------------------------------------
     * 情况3: 既不是数字也不是左括号 - 语法错误
     * ---------------------------------------------------------------
     * 
     * 这说明表达式语法不正确。例如：
     *   - 空表达式: ""
     *   - 缺少操作数: "2 +"
     *   - 两个运算符连续: "2 ++ 3"
     */
    parserSetError(parser, CALC_ERROR_UNEXPECTED_TOKEN, parser->lexer.current.start_pos);
    return 0.0;
}

/**
 * @brief 解析项（Term）
 * 
 * 项由因子和乘除运算符组成：Factor { (*|/) Factor }
 * 
 * ## 优先级说明
 * 乘除运算的优先级高于加减运算。在递归下降解析中：
 *   - parseExpression() 调用 parseTerm()
 *   - parseTerm() 负责处理乘除
 *   - 所以乘除会在加减之前被计算
 * 
 * ## 算法：左递归 + 循环
 * 
 * 这种模式称为"左递归"（Left Recursion）：
 *   - 先解析左边的因子（left = parseFactor()）
 *   - 如果遇到乘除运算符，保存运算符和位置
 *   - 读取右边的因子
 *   - 执行运算，更新 left
 *   - 重复直到遇到非乘除的 token
 * 
 * ### 示例解析 "2 * 3 * 4"
 * 
 *   第1次循环:
 *   ├─ left = 2
 *   ├─ 遇到 '*'
 *   ├─ right = 3
 *   └─ left = 2 * 3 = 6
 * 
 *   第2次循环:
 *   ├─ left = 6
 *   ├─ 遇到 '*'
 *   ├─ right = 4
 *   └─ left = 6 * 4 = 24
 * 
 *   第3次循环:
 *   └─ 遇到 '+'（不是乘除），退出循环
 * 
 * @param parser 解析器实例
 * @return 计算结果
 */
static double parserParseTerm(Parser* parser)
{
    /* 解析第一个因子作为初始值 */
    double left = parserParseFactor(parser);

    /* 循环处理连续的乘除运算
     * 
     * 循环条件：
     *   - 当前没有错误
     *   - 当前 token 是 '*' 或 '/'
     */
    while (parser->err == CALC_OK &&
           (parser->lexer.current.type == TOKEN_MUL ||
            parser->lexer.current.type == TOKEN_DIV)) {
        
        /* 保存运算符类型和位置（位置用于除零错误报告） */
        const TokenType op = parser->lexer.current.type;
        const size_t op_pos = parser->lexer.current.start_pos;
        
        /* 消费运算符，获取下一个 token */
        parserNextToken(parser);
        
        /* 检查错误 */
        if (parser->err != CALC_OK) {
            return 0.0;
        }

        /* 解析右边的因子 */
        {
            const double right = parserParseFactor(parser);
            
            /* 检查解析是否出错 */
            if (parser->err != CALC_OK) {
                return 0.0;
            }

            /* 执行运算 */
            if (op == TOKEN_MUL) {
                /* 乘法 */
                const double before = left;
                left *= right;
                
                parser->step_index++;
                parserEmitStep(parser, "%.10g * %.10g = %.10g", before, right, left);
            } else {
                /* 除法 - 需要检查除零 */
                if (fpclassify(right) == FP_ZERO) {
                    /* 设置除零错误，记录运算符位置 */
                    parserSetError(parser, CALC_ERROR_DIV_BY_ZERO, op_pos);
                    return 0.0;
                }
                
                {
                    const double before = left;
                    left /= right;
                    
                    parser->step_index++;
                    parserEmitStep(parser, "%.10g / %.10g = %.10g", before, right, left);
                }
            }
        }
    }

    return left;
}

/**
 * @brief 解析表达式（Expression）
 * 
 * 表达式由项和加减运算符组成：Term { (+|-) Term }
 * 
 * ## 与 parseTerm 的关系
 * 
 * 这里体现了递归下降解析器的层次结构：
 * 
 *   Expression
 *      │
 *      ├── parseTerm()  ← 处理乘除（优先级高）
 *      │
 *      └── while (+或-) {
 *             parseTerm()  ← 每次循环处理一个加或减
 *          }
 * 
 * ## 运算符关联性（Associativity）
 * 
 * 这种从左到右的循环处理方式实现了"左结合性"：
 *   - 2 - 3 - 4 = (2 - 3) - 4 = -5 （不是 2 - (3 - 4) = 3）
 *   - 2 * 3 * 4 = (2 * 3) * 4 = 24 （不是 2 * (3 * 4) = 24，结果相同但过程不同）
 * 
 * ### 减法的左结合性示例 "8 - 3 - 2"
 * 
 *   初始: left = parseTerm() = 8
 *   
 *   第1次循环:
 *   ├─ 遇到 '-'
 *   ├─ right = parseTerm() = 3
 *   └─ left = 8 - 3 = 5
 * 
 *   第2次循环:
 *   ├─ 遇到 '-'
 *   ├─ right = parseTerm() = 2
 *   └─ left = 5 - 2 = 3
 * 
 *   退出循环，结果 = 3 = (8-3)-2 ✓
 * 
 * @param parser 解析器实例
 * @return 计算结果
 */
static double parserParseExpression(Parser* parser)
{
    /* 解析第一个项作为初始值 */
    double left = parserParseTerm(parser);

    /* 循环处理连续的加减运算
     * 
     * 循环条件：
     *   - 当前没有错误
     *   - 当前 token 是 '+' 或 '-'
     */
    while (parser->err == CALC_OK &&
           (parser->lexer.current.type == TOKEN_PLUS ||
            parser->lexer.current.type == TOKEN_MINUS)) {
        
        /* 保存运算符类型 */
        const TokenType op = parser->lexer.current.type;
        
        /* 消费运算符，获取下一个 token */
        parserNextToken(parser);
        
        /* 检查错误 */
        if (parser->err != CALC_OK) {
            return 0.0;
        }

        /* 解析右边的项 */
        {
            const double right = parserParseTerm(parser);
            
            /* 检查解析是否出错 */
            if (parser->err != CALC_OK) {
                return 0.0;
            }
            
            /* 执行运算 */
            {
                const double before = left;
                left = (op == TOKEN_PLUS) ? (left + right) : (left - right);
                
                parser->step_index++;
                parserEmitStep(parser, "%.10g %c %.10g = %.10g",
                               before, op == TOKEN_PLUS ? '+' : '-', right, left);
            }
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
 * 这是解析器的公共入口函数。调用流程：
 *   1. 初始化解析器状态
 *   2. 读取第一个 token
 *   3. 调用 parseExpression() 解析并计算
 *   4. 检查是否所有 token 都被消费（防止尾部有垃圾）
 *   5. 返回结果或错误
 * 
 * ## 关于递归深度
 * 
 * 通过 options->max_recursion_depth 可以限制括号嵌套层数。
 * 这对于处理用户输入非常重要，可以防止：
 *   - 恶意输入如 "((((...))))" 导致栈溢出
 *   - 意外的死循环解析
 * 
 * @param expression 要计算的表达式字符串
 * @param options 评估选项（可为 NULL，使用默认选项）
 * @param result 输出参数，存储计算结果
 * @param err_pos 输出参数，存储错误位置（可为 NULL）
 * @return 错误码，CALC_OK 表示成功
 */
CalcError parserEvaluateExpression(const char* expression,
                                   const CalcEvalOptions* options,
                                   double* result,
                                   size_t* err_pos)
{
    Parser parser;
    double value;

    /* 初始化词法分析器 */
    lexerInit(&parser.lexer, expression);
    
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
        /* 自定义最大递归深度（如果非零） */
        if (options->max_recursion_depth != 0U) {
            parser.max_depth = options->max_recursion_depth;
        }
        
        /* 步骤回调设置 */
        parser.measure_step_time = options->measure_step_time;
        parser.on_step = options->on_step;
        parser.step_user_data = options->user_data;
    }

    /* 如果启用时间测量，记录起始时间 */
    if (parser.measure_step_time) {
        parser.last_step_clock = clock();
    }

    /* 读取第一个 token，准备开始解析 */
    parserNextToken(&parser);
    
    /* 执行表达式解析
     * 
     * 注意：这里只是调用了 parseExpression()，
     * 它会递归调用 parseTerm() 和 parseFactor()。
     */
    value = parserParseExpression(&parser);

    /* ---------------------------------------------------------------
     * 语法检查：确保所有 token 都被消费
     * ---------------------------------------------------------------
     * 
     * 这是很重要的检查！考虑以下情况：
     *   - 输入 "2 + 3 4"  （多了一个数字 4）
     *   - 词法分析: [2, +, 3, 4] （都是有效 token）
     *   - parseExpression 会消费 [2, +, 3]，然后遇到 [4]
     *   - 此时 lexer.current.type = TOKEN_NUMBER（不是 TOKEN_END）
     *   - 这说明有多余的内容，应该报错
     */
    if (parser.err == CALC_OK && parser.lexer.current.type != TOKEN_END) {
        parserSetError(&parser, CALC_ERROR_SYNTAX, parser.lexer.current.start_pos);
    }

    /* ---------------------------------------------------------------
     * 返回结果或错误
     * --------------------------------------------------------------- */
    if (parser.err != CALC_OK) {
        if (err_pos != NULL) {
            *err_pos = parser.err_pos;
        }
        return parser.err;
    }

    /* 成功：设置结果 */
    *result = value;
    if (err_pos != NULL) {
        *err_pos = 0;
    }
    return CALC_OK;
}
