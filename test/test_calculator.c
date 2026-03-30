/*******************************************************************************
 * test_calculator.c - 计算器单元测试
 *
 * 编译示例:
 *   gcc -o test_calculator test/test_calculator.c src/calculator.c src/lexer.c src/parser.c src/logger.c -Iinclude -lm
 *
 * 运行示例:
 *   ./test_calculator
 *   ./test_calculator --suite=error
 *   ./test_calculator --filter=非法字符
 ******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/calculator.h"
#include "../include/command.h"

#define EPSILON 1e-6

typedef struct {
    const char* name;
    const char* expression;
    double expected;
} SuccessCase;

typedef struct {
    const char* name;
    const char* expression;
    CalcError expected_error;
    int exact_match;
} ErrorCase;

typedef enum {
    SUITE_SUCCESS = 1 << 0,
    SUITE_ERROR = 1 << 1,
    SUITE_API = 1 << 2,
    SUITE_ALL = SUITE_SUCCESS | SUITE_ERROR | SUITE_API
} SuiteMask;

static const SuccessCase g_success_cases[] = {
    {"加法", "1+2", 3.0},
    {"减法", "10-3", 7.0},
    {"乘法", "2*3", 6.0},
    {"除法", "7/2", 3.5},
    {"优先级1", "2+3*4", 14.0},
    {"优先级2", "(2+3)*4", 20.0},
    {"负数1", "-5+3", -2.0},
    {"负数2", "(-1)*(-1)", 1.0},
    {"小数1", "0.5+0.25", 0.75},
    {"小数2", ".5 + .5", 1.0},
    {"科学计数法", "1e2+3", 103.0},
    {"微小非零除数", "1/0.0000000000001", 10000000000000.0},
    {"链式", "100/2/5", 10.0},
    {"复杂表达式", "((1+2)*(3-1))+5", 11.0},
    {"空格容忍", "  3 + 4 * 2  ", 11.0}
};

static const ErrorCase g_error_cases[] = {
    {"空串", "", CALC_ERROR_NULL_EXPR, 1},
    {"全空白", "   ", CALC_ERROR_UNEXPECTED_TOKEN, 1},
    {"直接除零", "1/0", CALC_ERROR_DIV_BY_ZERO, 1},
    {"间接除零", "1/(2-2)", CALC_ERROR_DIV_BY_ZERO, 1},
    {"非法字符", "1+a", CALC_ERROR_INVALID_CHAR, 1},
    {"双小数点", "1..2", CALC_ERROR_INVALID_CHAR, 0},
    {"缺少右括号", "(1+2", CALC_ERROR_MISSING_RPAREN, 1},
    {"多余右括号", "(1+2))", CALC_ERROR_SYNTAX, 1},
    {"尾部运算符", "1+2+", CALC_ERROR_UNEXPECTED_TOKEN, 1},
    {"孤立右括号", ")", CALC_ERROR_UNEXPECTED_TOKEN, 1},
    {"多余数字", "1 2", CALC_ERROR_SYNTAX, 1}
};

static const char* g_filter = NULL;
static SuiteMask g_suite_mask = SUITE_ALL;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_tests_selected = 0;
static int g_trace_calls = 0;

static int almost_equal(double lhs, double rhs) {
    const double diff = fabs(lhs - rhs);
    const double scale = fmax(1.0, fmax(fabs(lhs), fabs(rhs)));
    return diff <= EPSILON * scale;
}

static void trace_counter_cb(void* user_data, const CalcStepInfo* step) {
    (void)user_data;
    (void)step;
    g_trace_calls++;
}

static void fail_case(const char* name, const char* expression, const char* reason) {
    printf("  FAIL: %s\n", name);
    printf("    Expression: \"%s\"\n", expression ? expression : "(null)");
    printf("    Reason: %s\n", reason);
    g_tests_failed++;
}

static void pass_case(const char* name, const char* detail) {
    printf("  PASS: %s (%s)\n", name, detail);
    g_tests_passed++;
}

static int case_matches_filter(const char* name, const char* expression) {
    if (g_filter == NULL || *g_filter == '\0') {
        return 1;
    }
    if (name != NULL && strstr(name, g_filter) != NULL) {
        return 1;
    }
    if (expression != NULL && strstr(expression, g_filter) != NULL) {
        return 1;
    }
    return 0;
}

static int parse_suite(const char* value, SuiteMask* out_mask) {
    if (strcmp(value, "all") == 0) {
        *out_mask = SUITE_ALL;
        return 1;
    }
    if (strcmp(value, "success") == 0) {
        *out_mask = SUITE_SUCCESS;
        return 1;
    }
    if (strcmp(value, "error") == 0) {
        *out_mask = SUITE_ERROR;
        return 1;
    }
    if (strcmp(value, "api") == 0) {
        *out_mask = SUITE_API;
        return 1;
    }
    return 0;
}

static void print_usage(const char* prog) {
    printf("Usage: %s [--suite=all|success|error|api] [--filter=TEXT] [--list] [--help]\n", prog);
}

static void list_cases(void) {
    size_t i;
    printf("Suites: all, success, error, api\n");
    printf("\n[success]\n");
    for (i = 0; i < sizeof(g_success_cases) / sizeof(g_success_cases[0]); ++i) {
        printf("  - %s | %s\n", g_success_cases[i].name, g_success_cases[i].expression);
    }
    printf("\n[error]\n");
    for (i = 0; i < sizeof(g_error_cases) / sizeof(g_error_cases[0]); ++i) {
        printf("  - %s | %s\n", g_error_cases[i].name, g_error_cases[i].expression);
    }
    printf("\n[api]\n");
    printf("  - result=NULL\n");
    printf("  - expression=NULL\n");
    printf("  - recursion-limit\n");
    printf("  - evaluate-step-hook\n");
    printf("  - command-dispatch\n");
}

static int parse_args(int argc, char** argv) {
    int i;
    for (i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(argv[0]);
            return 1;
        }
        if (strcmp(arg, "--list") == 0) {
            list_cases();
            return 1;
        }
        if (strncmp(arg, "--filter=", 9) == 0) {
            g_filter = arg + 9;
            continue;
        }
        if (strcmp(arg, "--filter") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --filter\n");
                return -1;
            }
            g_filter = argv[++i];
            continue;
        }
        if (strncmp(arg, "--suite=", 8) == 0) {
            if (!parse_suite(arg + 8, &g_suite_mask)) {
                fprintf(stderr, "Invalid suite: %s\n", arg + 8);
                return -1;
            }
            continue;
        }
        if (strcmp(arg, "--suite") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --suite\n");
                return -1;
            }
            if (!parse_suite(argv[++i], &g_suite_mask)) {
                fprintf(stderr, "Invalid suite: %s\n", argv[i]);
                return -1;
            }
            continue;
        }

        fprintf(stderr, "Unknown argument: %s\n", arg);
        print_usage(argv[0]);
        return -1;
    }
    return 0;
}

static void run_success_case(const SuccessCase* tc) {
    double result = 0.0;
    size_t err_pos = 0;
    CalcError err = evaluate(tc->expression, NULL, &result, &err_pos);

    if (err != CALC_OK) {
        char buffer[160];
        snprintf(buffer, sizeof(buffer), "expected CALC_OK, got %d at pos %zu", err, err_pos);
        fail_case(tc->name, tc->expression, buffer);
        return;
    }

    if (!almost_equal(result, tc->expected)) {
        char buffer[160];
        snprintf(buffer, sizeof(buffer), "expected %.8f, got %.8f", tc->expected, result);
        fail_case(tc->name, tc->expression, buffer);
        return;
    }

    {
        char detail[96];
        snprintf(detail, sizeof(detail), "result=%.6f", result);
        pass_case(tc->name, detail);
    }
}

static void run_error_case(const ErrorCase* tc) {
    double result = 0.0;
    size_t err_pos = 0;
    CalcError err = evaluate(tc->expression, NULL, &result, &err_pos);

    if (tc->exact_match) {
        if (err != tc->expected_error) {
            char buffer[160];
            snprintf(buffer, sizeof(buffer), "expected error %d, got %d at pos %zu",
                     tc->expected_error, err, err_pos);
            fail_case(tc->name, tc->expression, buffer);
            return;
        }
    } else if (err == CALC_OK) {
        fail_case(tc->name, tc->expression, "expected non-OK error, got CALC_OK");
        return;
    }

    {
        char detail[96];
        if (tc->exact_match) {
            snprintf(detail, sizeof(detail), "error=%d pos=%zu", err, err_pos);
        } else {
            snprintf(detail, sizeof(detail), "non-OK error=%d pos=%zu", err, err_pos);
        }
        pass_case(tc->name, detail);
    }
}

static void run_success_suite(void) {
    size_t i;
    printf("\n=== 成功用例 ===\n");
    for (i = 0; i < sizeof(g_success_cases) / sizeof(g_success_cases[0]); ++i) {
        const SuccessCase* tc = &g_success_cases[i];
        if (!case_matches_filter(tc->name, tc->expression)) {
            continue;
        }
        g_tests_selected++;
        run_success_case(tc);
    }
}

static void run_error_suite(void) {
    size_t i;
    printf("\n=== 错误用例 ===\n");
    for (i = 0; i < sizeof(g_error_cases) / sizeof(g_error_cases[0]); ++i) {
        const ErrorCase* tc = &g_error_cases[i];
        if (!case_matches_filter(tc->name, tc->expression)) {
            continue;
        }
        g_tests_selected++;
        run_error_case(tc);
    }
}

static void run_api_contract_suite(void) {
    size_t err_pos = 0;
    CalcError err;

    printf("\n=== API 合约用例 ===\n");

    if (case_matches_filter("result=NULL", "1+2,NULL")) {
        g_tests_selected++;
        err = evaluate("1+2", NULL, NULL, &err_pos);
        if (err == CALC_ERROR_NULL_EXPR) {
            pass_case("result=NULL", "returns CALC_ERROR_NULL_EXPR");
        } else {
            fail_case("result=NULL", "1+2", "did not return CALC_ERROR_NULL_EXPR");
        }
    }

    if (case_matches_filter("expression=NULL", "NULL,NULL")) {
        g_tests_selected++;
        err = evaluate(NULL, NULL, NULL, &err_pos);
        if (err == CALC_ERROR_NULL_EXPR) {
            pass_case("expression=NULL", "returns CALC_ERROR_NULL_EXPR");
        } else {
            fail_case("expression=NULL", "(null)", "did not return CALC_ERROR_NULL_EXPR");
        }
    }

    if (case_matches_filter("recursion-limit", "((((...))))")) {
        const size_t depth = 320;
        const size_t expr_len = depth * 2 + 1;
        double value = 0.0;
        char* expr = (char*)malloc(expr_len + 1);
        g_tests_selected++;

        if (expr == NULL) {
            fail_case("recursion-limit", "(oom)", "malloc failed");
            return;
        }

        memset(expr, '(', depth);
        expr[depth] = '1';
        memset(expr + depth + 1, ')', depth);
        expr[expr_len] = '\0';

        err = evaluate(expr, NULL, &value, &err_pos);

        if (err == CALC_ERROR_RECURSION_LIMIT) {
            pass_case("recursion-limit", "returns CALC_ERROR_RECURSION_LIMIT");
        } else {
            char detail[128];
            snprintf(detail, sizeof(detail), "expected %d, got %d at pos %zu",
                     CALC_ERROR_RECURSION_LIMIT, err, err_pos);
            fail_case("recursion-limit", expr, detail);
        }

        free(expr);
    }

    if (case_matches_filter("evaluate-step-hook", "2+3*4")) {
        CalcEvalOptions options;
        double value = 0.0;
        g_tests_selected++;
        g_trace_calls = 0;
        calcEvalOptionsInit(&options);
        options.on_step = trace_counter_cb;
        options.measure_step_time = true;
        err = evaluate("2+3*4", &options, &value, &err_pos);
        if (err == CALC_OK && almost_equal(value, 14.0) && g_trace_calls > 0) {
            pass_case("evaluate-step-hook", "returns CALC_OK and emits step events");
        } else {
            char detail[160];
            snprintf(detail, sizeof(detail),
                     "unexpected result: err=%d value=%.10g trace_calls=%d",
                     err, value, g_trace_calls);
            fail_case("evaluate-step-hook", "2+3*4", detail);
        }
    }

    if (case_matches_filter("command-dispatch", "show process")) {
        CommandState state;
        CommandResult cmd;
        g_tests_selected++;
        commandStateInit(&state);

        cmd = commandDispatch("show process", &state);
        if (cmd != COMMAND_RESULT_HANDLED || !state.show_process) {
            fail_case("command-dispatch", "show process", "failed to enable process mode");
            return;
        }

        cmd = commandDispatch("hide process", &state);
        if (cmd != COMMAND_RESULT_HANDLED || state.show_process) {
            fail_case("command-dispatch", "hide process", "failed to disable process mode");
            return;
        }

        cmd = commandDispatch("show help", &state);
        if (cmd != COMMAND_RESULT_HANDLED) {
            fail_case("command-dispatch", "show help", "failed to execute help command");
            return;
        }

        cmd = commandDispatch("show unknown", &state);
        if (cmd != COMMAND_RESULT_ERROR) {
            fail_case("command-dispatch", "show unknown", "unknown command did not return error");
            return;
        }

        cmd = commandDispatch("2+3", &state);
        if (cmd != COMMAND_RESULT_NOT_COMMAND) {
            fail_case("command-dispatch", "2+3", "expression was incorrectly treated as command");
            return;
        }

        pass_case("command-dispatch", "dispatcher works as expected");
    }
}

int main(int argc, char** argv) {
    int parse_result = parse_args(argc, argv);
    if (parse_result > 0) {
        return 0;
    }
    if (parse_result < 0) {
        return 2;
    }

    printf("========================================\n");
    printf("     计算器单元测试\n");
    printf("========================================\n");

    if (g_suite_mask & SUITE_SUCCESS) {
        run_success_suite();
    }
    if (g_suite_mask & SUITE_ERROR) {
        run_error_suite();
    }
    if (g_suite_mask & SUITE_API) {
        run_api_contract_suite();
    }

    printf("\n========================================\n");
    printf("     测试结果汇总\n");
    printf("========================================\n");
    printf("筛选后执行: %d\n", g_tests_selected);
    printf("通过: %d\n", g_tests_passed);
    printf("失败: %d\n", g_tests_failed);
    printf("总计: %d\n", g_tests_passed + g_tests_failed);

    if (g_tests_selected == 0) {
        printf("\n筛选后无匹配测试。\n");
        return 2;
    }
    if (g_tests_failed > 0) {
        printf("\n有 %d 个测试失败。\n", g_tests_failed);
        return 1;
    }

    printf("\n所有测试通过。\n");
    return 0;
}
