/**
 * @file debug.c
 * @brief 调试系统运行时实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "debug.h"

/* ========================================================================
 * 全局状态
 * ======================================================================== */

int      g_debug_level = DEBUG_LEVEL_NONE;
unsigned g_debug_modules = 0;
bool     g_debug_color = true;
FILE    *g_debug_output = NULL;

/* ========================================================================
 * 内部辅助
 * ======================================================================== */

/**
 * @brief 检测是否为终端（用于自动关闭彩色输出）
 */
static bool is_terminal(void) {
#if defined(_WIN32) || defined(_WIN64)
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

/**
 * @brief 获取级别的标签字符串
 */
static const char *level_tag(int level) {
    switch (level) {
        case DEBUG_LEVEL_ERROR: return "ERROR";
        case DEBUG_LEVEL_WARN:  return "WARN ";
        case DEBUG_LEVEL_INFO:  return "INFO ";
        case DEBUG_LEVEL_DEBUG: return "DEBUG";
        case DEBUG_LEVEL_TRACE: return "TRACE";
        default:                 return "?????";
    }
}

/**
 * @brief 获取级别的颜色代码
 */
static const char *level_color(int level) {
    if (!g_debug_color) return "";
    switch (level) {
        case DEBUG_LEVEL_ERROR: return DEBUG_ANSI_RED DEBUG_ANSI_BOLD;
        case DEBUG_LEVEL_WARN:  return DEBUG_ANSI_YELLOW;
        case DEBUG_LEVEL_INFO:  return DEBUG_ANSI_GREEN;
        case DEBUG_LEVEL_DEBUG: return DEBUG_ANSI_BLUE;
        case DEBUG_LEVEL_TRACE: return DEBUG_ANSI_CYAN;
        default:                return DEBUG_ANSI_RESET;
    }
}

/**
 * @brief 获取模块的标签字符串
 */
static const char *module_tag(unsigned module) {
    switch (module) {
        case DEBUG_MODULE_LEXER:  return "[LEXER] ";
        case DEBUG_MODULE_PARSER: return "[PARSER]";
        case DEBUG_MODULE_CALC:   return "[CALC]  ";
        case DEBUG_MODULE_MAIN:   return "[MAIN]  ";
        case DEBUG_MODULE_CORE:   return "[CORE]  ";
        default:                  return "[       ]";
    }
}

/**
 * @brief 获取时间戳
 */
static void timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, len, "%H:%M:%S", tm_info);
}

/* ========================================================================
 * 公开 API 实现
 * ======================================================================== */

void debug_init(int level, unsigned module) {
    g_debug_level = level;
    g_debug_modules = module;
    g_debug_color = is_terminal();
    g_debug_output = NULL;
}

bool debug_parse_args(int argc, char *argv[]) {
    bool activated = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        /* -d, --debug: 开启调试 */
        if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
            if (g_debug_level == DEBUG_LEVEL_NONE) {
                g_debug_level = DEBUG_DEFAULT_LEVEL;
            }
            g_debug_modules = DEBUG_DEFAULT_MODULE;
            activated = true;
        }
        /* -v: 详细模式（等于 --debug） */
        else if (strcmp(arg, "-v") == 0) {
            g_debug_level = DEBUG_LEVEL_INFO;
            g_debug_modules = DEBUG_DEFAULT_MODULE;
            activated = true;
        }
        /* -l, --debug-level=N: 设置级别 */
        else if (strncmp(arg, "-l", 2) == 0) {
            int lvl = atoi(arg + 2);
            if (lvl >= 0 && lvl <= 5) {
                g_debug_level = lvl;
                activated = true;
            }
        }
        else if (strncmp(arg, "--debug-level=", 14) == 0) {
            int lvl = atoi(arg + 14);
            if (lvl >= 0 && lvl <= 5) {
                g_debug_level = lvl;
                activated = true;
            }
        }
        /* -m, --debug-module=NAME: 设置模块 */
        else if (strncmp(arg, "-m", 2) == 0) {
            const char *mod = arg + 2;
            if (strcmp(mod, "lexer") == 0) {
                g_debug_modules = DEBUG_MODULE_LEXER;
            } else if (strcmp(mod, "parser") == 0) {
                g_debug_modules = DEBUG_MODULE_PARSER;
            } else if (strcmp(mod, "calc") == 0) {
                g_debug_modules = DEBUG_MODULE_CALC;
            } else if (strcmp(mod, "main") == 0) {
                g_debug_modules = DEBUG_MODULE_MAIN;
            } else if (strcmp(mod, "all") == 0) {
                g_debug_modules = DEBUG_MODULE_ALL;
            }
            activated = true;
        }
        else if (strncmp(arg, "--debug-module=", 14) == 0) {
            const char *mod = arg + 14;
            if (strcmp(mod, "lexer") == 0) {
                g_debug_modules = DEBUG_MODULE_LEXER;
            } else if (strcmp(mod, "parser") == 0) {
                g_debug_modules = DEBUG_MODULE_PARSER;
            } else if (strcmp(mod, "calc") == 0) {
                g_debug_modules = DEBUG_MODULE_CALC;
            } else if (strcmp(mod, "main") == 0) {
                g_debug_modules = DEBUG_MODULE_MAIN;
            } else if (strcmp(mod, "all") == 0) {
                g_debug_modules = DEBUG_MODULE_ALL;
            }
            activated = true;
        }
    }

    if (activated) {
        g_debug_color = is_terminal();
    }

    return activated;
}

void debug_set_level(int level) {
    g_debug_level = level;
}

int debug_get_level(void) {
    return g_debug_level;
}

void debug_set_modules(unsigned module) {
    g_debug_modules = module;
}

unsigned debug_get_modules(void) {
    return g_debug_modules;
}

void debug_enable_color(bool enable) {
    g_debug_color = enable;
}

bool debug_is_color_enabled(void) {
    return g_debug_color;
}

void debug_set_output(FILE *fp) {
    g_debug_output = fp;
}

/* ========================================================================
 * 核心输出函数
 * ======================================================================== */

void debug_log(int level, unsigned module,
               const char *file, int line, const char *func,
               const char *format, ...) {
    FILE *out = g_debug_output ? g_debug_output : stderr;

    char time_buf[16];
    timestamp(time_buf, sizeof(time_buf));

    const char *color = level_color(level);
    const char *reset = g_debug_color ? DEBUG_ANSI_RESET : "";

    /* 输出格式：[TIME] [LEVEL] [MODULE] func(): message */
    fprintf(out, "[%s] %s%s%s %s %s(): ",
            time_buf,
            color,
            level_tag(level),
            reset,
            module_tag(module),
            func);

    va_list args;
    va_start(args, format);
    vfprintf(out, format, args);
    va_end(args);

    fprintf(out, " (at %s:%d)\n", file, line);
    fflush(out);
}

void debug_trace(const char *file, int line, const char *func,
                 unsigned module, bool is_entry, double result) {
    FILE *out = g_debug_output ? g_debug_output : stderr;

    const char *color = level_color(DEBUG_LEVEL_TRACE);
    const char *reset = g_debug_color ? DEBUG_ANSI_RESET : "";
    const char *tag = is_entry ? "ENTER" : "EXIT";
    const char *fname = file;
    const char *p = strrchr(file, '/');
    if (p) fname = p + 1;
#if defined(_WIN32) || defined(_WIN64)
    p = strrchr(file, '\\');
    if (p) fname = p + 1;
#endif
    (void)module;  /* 未使用，但保留参数以匹配宏签名 */

    fprintf(out, "%s[TRACE]%s %s %s:%d %s()",
            color, reset,
            tag,
            fname,
            line,
            func);

    if (!is_entry) {
        fprintf(out, " => %.10g", result);
    }

    fprintf(out, "\n");
    fflush(out);
}
