#include "parser.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "lexer.h"

#define PARSER_DEFAULT_MAX_RECURSION 256U

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

static double parserParseExpression(Parser* parser);

static void parserSetError(Parser* parser, CalcError err, size_t err_pos)
{
    if (parser->err == CALC_OK) {
        parser->err = err;
        parser->err_pos = err_pos;
    }
}

static void parserNextToken(Parser* parser)
{
    lexerNextToken(&parser->lexer);
    if (parser->lexer.err != CALC_OK) {
        parserSetError(parser, parser->lexer.err, parser->lexer.err_pos);
    }
}

static void parserEmitStep(Parser* parser, const char* fmt, ...)
{
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

static double parserParseFactor(Parser* parser)
{
    int sign = 1;

    if (parser->err != CALC_OK) {
        return 0.0;
    }

    while (parser->lexer.current.type == TOKEN_PLUS ||
           parser->lexer.current.type == TOKEN_MINUS) {
        const TokenType unary = parser->lexer.current.type;
        if (unary == TOKEN_MINUS) {
            sign = -sign;
        }
        parserNextToken(parser);
        if (parser->err != CALC_OK) {
            return 0.0;
        }
        parser->step_index++;
        parserEmitStep(parser, "读取一元运算符 '%c'", unary == TOKEN_MINUS ? '-' : '+');
    }

    if (parser->lexer.current.type == TOKEN_NUMBER) {
        const double raw = parser->lexer.current.value;
        const double value = sign * raw;
        parser->step_index++;
        parserEmitStep(parser, "读取数字 %.10g%s，结果 %.10g",
                       raw, (sign < 0) ? " 并应用一元负号" : "", value);
        parserNextToken(parser);
        return value;
    }

    if (parser->lexer.current.type == TOKEN_LPAREN) {
        const size_t lparen_pos = parser->lexer.current.start_pos;
        parserNextToken(parser);

        if (parser->depth >= parser->max_depth) {
            parserSetError(parser, CALC_ERROR_RECURSION_LIMIT, lparen_pos);
            return 0.0;
        }

        parser->depth++;
        {
            const double inner = parserParseExpression(parser);
            parser->depth--;

            if (parser->err != CALC_OK) {
                return 0.0;
            }

            if (parser->lexer.current.type != TOKEN_RPAREN) {
                parserSetError(parser, CALC_ERROR_MISSING_RPAREN,
                               parser->lexer.current.start_pos);
                return 0.0;
            }

            parserNextToken(parser);
            if (sign < 0) {
                parser->step_index++;
                parserEmitStep(parser, "对括号表达式应用一元负号，结果 %.10g", -inner);
                return -inner;
            }
            return inner;
        }
    }

    parserSetError(parser, CALC_ERROR_UNEXPECTED_TOKEN, parser->lexer.current.start_pos);
    return 0.0;
}

static double parserParseTerm(Parser* parser)
{
    double left = parserParseFactor(parser);

    while (parser->err == CALC_OK &&
           (parser->lexer.current.type == TOKEN_MUL ||
            parser->lexer.current.type == TOKEN_DIV)) {
        const TokenType op = parser->lexer.current.type;
        const size_t op_pos = parser->lexer.current.start_pos;
        parserNextToken(parser);
        if (parser->err != CALC_OK) {
            return 0.0;
        }

        {
            const double right = parserParseFactor(parser);
            if (parser->err != CALC_OK) {
                return 0.0;
            }

            if (op == TOKEN_MUL) {
                const double before = left;
                left *= right;
                parser->step_index++;
                parserEmitStep(parser, "%.10g * %.10g = %.10g", before, right, left);
            } else {
                if (fpclassify(right) == FP_ZERO) {
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

static double parserParseExpression(Parser* parser)
{
    double left = parserParseTerm(parser);

    while (parser->err == CALC_OK &&
           (parser->lexer.current.type == TOKEN_PLUS ||
            parser->lexer.current.type == TOKEN_MINUS)) {
        const TokenType op = parser->lexer.current.type;
        parserNextToken(parser);
        if (parser->err != CALC_OK) {
            return 0.0;
        }

        {
            const double right = parserParseTerm(parser);
            if (parser->err != CALC_OK) {
                return 0.0;
            }
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

CalcError parserEvaluateExpression(const char* expression,
                                   const CalcEvalOptions* options,
                                   double* result,
                                   size_t* err_pos)
{
    Parser parser;
    double value;

    lexerInit(&parser.lexer, expression);
    parser.err = CALC_OK;
    parser.err_pos = 0;
    parser.depth = 0U;
    parser.step_index = 0U;
    parser.max_depth = PARSER_DEFAULT_MAX_RECURSION;
    parser.measure_step_time = false;
    parser.on_step = NULL;
    parser.step_user_data = NULL;
    parser.last_step_clock = (clock_t)-1;

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

    parserNextToken(&parser);
    value = parserParseExpression(&parser);

    if (parser.err == CALC_OK && parser.lexer.current.type != TOKEN_END) {
        parserSetError(&parser, CALC_ERROR_SYNTAX, parser.lexer.current.start_pos);
    }

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
