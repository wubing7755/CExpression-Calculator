#include "parser.h"

#include <math.h>
#include "lexer.h"

#define PARSER_MAX_RECURSION 256U

typedef struct {
    Lexer lexer;
    CalcError err;
    size_t err_pos;
    unsigned depth;
    unsigned max_depth;
} Parser;

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

static double parserParseExpression(Parser* parser);

static double parserParseFactor(Parser* parser)
{
    int sign = 1;

    if (parser->err != CALC_OK) {
        return 0.0;
    }

    while (parser->lexer.current.type == TOKEN_PLUS ||
           parser->lexer.current.type == TOKEN_MINUS) {
        if (parser->lexer.current.type == TOKEN_MINUS) {
            sign = -sign;
        }
        parserNextToken(parser);
        if (parser->err != CALC_OK) {
            return 0.0;
        }
    }

    if (parser->lexer.current.type == TOKEN_NUMBER) {
        const double value = parser->lexer.current.value;
        parserNextToken(parser);
        return sign * value;
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
            return sign * inner;
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
                left *= right;
            } else {
                if (fpclassify(right) == FP_ZERO) {
                    parserSetError(parser, CALC_ERROR_DIV_BY_ZERO, op_pos);
                    return 0.0;
                }
                left /= right;
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
            left = (op == TOKEN_PLUS) ? (left + right) : (left - right);
        }
    }

    return left;
}

CalcError parserEvaluateExpression(const char* expression, double* result, size_t* err_pos)
{
    Parser parser;
    double value;

    lexerInit(&parser.lexer, expression);
    parser.err = CALC_OK;
    parser.err_pos = 0;
    parser.depth = 0U;
    parser.max_depth = PARSER_MAX_RECURSION;

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
