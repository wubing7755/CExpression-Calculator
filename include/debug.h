/**
 * @file debug.h
 * @brief CalcExpr 调试宏系统
 *
 * 功能特性：
 * - 全局开关 DEBUG_ENABLE（编译期控制，关闭时零开销）
 * - 模块控制（LEXER、PARSER、CALC、MAIN）
 * - 级别控制（ERROR、WARN、INFO、DEBUG、TRACE）
 * - 自动包含文件名、行号、函数名
 * - 彩色输出（可关闭）
 * - 命令行参数支持（--debug、--debug-level=N、-d）
 *
 * 使用方式：
 *
 * 1. 编译期开启（CMake）：
 *    cmake -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug ..
 *
 * 2. 运行时开启：
 *    ./calculator --debug
 *    ./calculator -d --debug-level=0
 *    ./calculator --debug-module=parser -d
 *
 * 3. 代码中使用：
 *    DEBUG_ERROR("除零错误 at %zu", pos);
 *    DEBUG_INFO("计算结果: %.10g", result);
 *    DEBUG_PARSER("Token: %s", tokenName(type));
 *    DEBUG_TRACE_ENTER(DEBUG_MODULE_PARSER);
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* ========================================================================
 * 编译期配置（默认关闭以保证零开销）
 * ======================================================================== */

/* 主开关：设置为 1 开启调试系统，设置 0 完全禁用（零开销） */
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 0
#endif

/* 默认调试级别（DEBUG_ENABLE=1 时使用） */
#ifndef DEBUG_DEFAULT_LEVEL
#define DEBUG_DEFAULT_LEVEL 3  /* INFO */
#endif

/* 默认调试模块（DEBUG_ENABLE=1 时使用） */
#ifndef DEBUG_DEFAULT_MODULE
#define DEBUG_DEFAULT_MODULE 0xFF  /* ALL */
#endif

/* ========================================================================
 * 调试模块定义（可组合，如 DEBUG_MODULE_LEXER | DEBUG_MODULE_PARSER）
 * ======================================================================== */

#define DEBUG_MODULE_CORE   (1U << 0)   /**< 核心模块 */
#define DEBUG_MODULE_LEXER  (1U << 1)   /**< 词法分析器 */
#define DEBUG_MODULE_PARSER (1U << 2)   /**< 解析器 */
#define DEBUG_MODULE_CALC   (1U << 3)   /**< 计算器 */
#define DEBUG_MODULE_MAIN   (1U << 4)   /**< 主程序 */
#define DEBUG_MODULE_ALL    0xFFFFFFFFU  /**< 所有模块 */

/* ========================================================================
 * 调试级别定义（数值越小越详细）
 * ======================================================================== */

typedef enum {
    DEBUG_LEVEL_NONE   = 0,  /**< 完全关闭 */
    DEBUG_LEVEL_ERROR  = 1,  /**< 仅错误 */
    DEBUG_LEVEL_WARN   = 2,  /**< 警告及以上 */
    DEBUG_LEVEL_INFO   = 3,  /**< 信息及以上 */
    DEBUG_LEVEL_DEBUG  = 4,  /**< 调试及以上 */
    DEBUG_LEVEL_TRACE  = 5   /**< 最详细（函数跟踪） */
} DebugLevel;

/* ========================================================================
 * ANSI 颜色定义
 * ======================================================================== */

#define DEBUG_ANSI_RESET   "\033[0m"
#define DEBUG_ANSI_RED     "\033[31m"
#define DEBUG_ANSI_YELLOW  "\033[33m"
#define DEBUG_ANSI_GREEN   "\033[32m"
#define DEBUG_ANSI_BLUE    "\033[34m"
#define DEBUG_ANSI_CYAN   "\033[36m"
#define DEBUG_ANSI_GRAY    "\033[90m"
#define DEBUG_ANSI_BOLD    "\033[1m"

/* ========================================================================
 * 编译器辅助宏
 * ======================================================================== */

#if defined(__GNUC__) || defined(__clang__)
    /** @brief 标记未使用的变量（消除警告） */
    #define DEBUG_UNUSED __attribute__((unused))
    /** @brief 提示编译器条件通常为真 */
    #define DEBUGlikely(x)   __builtin_expect(!!(x), 1)
    /** @brief 提示编译器条件通常为假 */
    #define DEBUGunlikely(x) __builtin_expect(!!(x), 0)
#else
    #define DEBUG_UNUSED
    #define DEBUGlikely(x)   (x)
    #define DEBUGunlikely(x) (x)
#endif

/* ========================================================================
 * 运行时状态声明（debug.c 实现）
 * ======================================================================== */

extern int      g_debug_level;        /**< 当前调试级别 */
extern unsigned g_debug_modules;      /**< 当前启用的模块位掩码 */
extern bool     g_debug_color;        /**< 是否启用彩色输出 */
extern FILE    *g_debug_output;       /**< 输出目标（NULL=stderr） */

/* ========================================================================
 * 运行时 API（debug.c 实现，仅在 DEBUG_ENABLE=1 时声明）
 * ======================================================================== */

#if DEBUG_ENABLE

/**
 * @brief 初始化调试系统
 * @param level  初始调试级别
 * @param module 初始启用的模块位掩码
 */
void debug_init(int level, unsigned module);

/**
 * @brief 解析命令行参数
 * @param argc  main() 的 argc
 * @param argv  main() 的 argv
 * @return true 如果开启了调试模式
 *
 * 支持的参数：
 *   -d, --debug          开启调试（默认级别）
 *   -v                   同 --debug
 *   -l, --debug-level=N  设置调试级别 (0-5)
 *   -m, --debug-module=M 设置模块 (lexer|parser|calc|main|all)
 */
bool debug_parse_args(int argc, char *argv[]);

/**
 * @brief 运行时控制函数
 * @{
 */
void debug_set_level(int level);
int  debug_get_level(void);
void debug_set_modules(unsigned module);
unsigned debug_get_modules(void);
void debug_enable_color(bool enable);
bool debug_is_color_enabled(void);
void debug_set_output(FILE *fp);
/** @} */

#endif /* DEBUG_ENABLE */

/* ========================================================================
 * 核心调试宏（当 DEBUG_ENABLE=0 时完全无开销）
 * ======================================================================== */

#if DEBUG_ENABLE

/* ---- 内部辅助函数 ---- */

void debug_log(int level, unsigned module,
               const char *file, int line, const char *func,
               const char *format, ...);

void debug_trace(const char *file, int line, const char *func,
                unsigned module, bool is_entry, double result);

/* ---- 主调试宏 ---- */

/**
 * @brief 模块+级别调试输出
 * @param level   DEBUG_LEVEL_* 值
 * @param module  DEBUG_MODULE_* 值
 * @param fmt     printf 格式字符串
 * @param ...     格式参数
 *
 * 示例：DEBUG_LOG(DEBUG_LEVEL_DEBUG, DEBUG_MODULE_PARSER, "Token: %s", name);
 */
#define DEBUG_LOG(level, module, fmt, ...) \
    do { \
        if (DEBUGunlikely(g_debug_level >= (level) && \
                        ((module) & g_debug_modules))) { \
            debug_log((level), (module), __FILE__, __LINE__, __func__, \
                     fmt, ##__VA_ARGS__); \
        } \
    } while (0)

/**
 * @brief 函数入口跟踪
 * @param module DEBUG_MODULE_* 值
 *
 * 示例：DEBUG_TRACE_ENTER(DEBUG_MODULE_PARSER);
 */
#define DEBUG_TRACE_ENTER(module) \
    do { \
        if (DEBUGunlikely(g_debug_level >= DEBUG_LEVEL_TRACE && \
                        ((module) & g_debug_modules))) { \
            debug_trace(__FILE__, __LINE__, __func__, (module), true, 0.0); \
        } \
    } while (0)

/**
 * @brief 函数出口跟踪
 * @param module DEBUG_MODULE_* 值
 * @param result 函数返回值
 *
 * 示例：DEBUG_TRACE_EXIT(DEBUG_MODULE_PARSER, result);
 */
#define DEBUG_TRACE_EXIT(module, result) \
    do { \
        if (DEBUGunlikely(g_debug_level >= DEBUG_LEVEL_TRACE && \
                        ((module) & g_debug_modules))) { \
            debug_trace(__FILE__, __LINE__, __func__, (module), false, (result)); \
        } \
    } while (0)

/* ---- 便捷宏（按级别，不指定模块） ---- */

/**
 * @brief 错误信息（红色）
 * @note 级别：DEBUG_LEVEL_ERROR
 */
#define DEBUG_ERROR(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_ERROR, DEBUG_MODULE_ALL, fmt, ##__VA_ARGS__)

/**
 * @brief 警告信息（黄色）
 * @note 级别：DEBUG_LEVEL_WARN
 */
#define DEBUG_WARN(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_WARN, DEBUG_MODULE_ALL, fmt, ##__VA_ARGS__)

/**
 * @brief 一般信息（绿色）
 * @note 级别：DEBUG_LEVEL_INFO
 */
#define DEBUG_INFO(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_INFO, DEBUG_MODULE_ALL, fmt, ##__VA_ARGS__)

/**
 * @brief 调试信息（蓝色）
 * @note 级别：DEBUG_LEVEL_DEBUG
 */
#define DEBUG_DEBUG(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_DEBUG, DEBUG_MODULE_ALL, fmt, ##__VA_ARGS__)

/**
 * @brief 跟踪信息（青色）
 * @note 级别：DEBUG_LEVEL_TRACE
 */
#define DEBUG_TRACE(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_TRACE, DEBUG_MODULE_ALL, fmt, ##__VA_ARGS__)

/* ---- 便捷宏（按模块，不指定级别） ---- */

/**
 * @brief Lexer 模块调试
 * @note 默认级别：DEBUG_LEVEL_TRACE
 */
#define DEBUG_LEXER(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_TRACE, DEBUG_MODULE_LEXER, fmt, ##__VA_ARGS__)

/**
 * @brief Parser 模块调试
 * @note 默认级别：DEBUG_LEVEL_DEBUG
 */
#define DEBUG_PARSER(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_DEBUG, DEBUG_MODULE_PARSER, fmt, ##__VA_ARGS__)

/**
 * @brief Calc 模块调试
 * @note 默认级别：DEBUG_LEVEL_DEBUG
 */
#define DEBUG_CALC(fmt, ...) \
    DEBUG_LOG(DEBUG_LEVEL_DEBUG, DEBUG_MODULE_CALC, fmt, ##__VA_ARGS__)

/* ---- 条件宏 ---- */

/**
 * @brief 条件调试块
 * @param level DEBUG_LEVEL_* 值
 * @param module DEBUG_MODULE_* 值
 * @param stmt   调试代码块
 *
 * 示例：
 *   DEBUG_IF(DEBUG_LEVEL_DEBUG, DEBUG_MODULE_PARSER) {
 *       printf("Current token: %s\n", tokenName(type));
 *   }
 */
#define DEBUG_IF(level, module, stmt) \
    do { \
        if (DEBUGunlikely(g_debug_level >= (level) && \
                        ((module) & g_debug_modules))) { \
            stmt; \
        } \
    } while (0)

#else /* DEBUG_ENABLE == 0 */

/* ---- 禁用状态：所有宏为空 ---- */

#define DEBUG_LOG(level, module, fmt, ...) ((void)0)
#define DEBUG_TRACE_ENTER(module)         ((void)0)
#define DEBUG_TRACE_EXIT(module, result)  ((void)0)
#define DEBUG_ERROR(fmt, ...)             ((void)0)
#define DEBUG_WARN(fmt, ...)              ((void)0)
#define DEBUG_INFO(fmt, ...)              ((void)0)
#define DEBUG_DEBUG(fmt, ...)             ((void)0)
#define DEBUG_TRACE(fmt, ...)             ((void)0)
#define DEBUG_LEXER(fmt, ...)            ((void)0)
#define DEBUG_PARSER(fmt, ...)           ((void)0)
#define DEBUG_CALC(fmt, ...)             ((void)0)
#define DEBUG_IF(level, module, stmt)    ((void)0)

/* ---- 禁用状态的桩函数（防止链接错误） ---- */

static inline void debug_init(int level, unsigned module) { (void)level; (void)module; }
static inline bool debug_parse_args(int argc, char *argv[]) { (void)argc; (void)argv; return false; }
static inline void debug_set_level(int level) { (void)level; }
static inline int  debug_get_level(void) { return 0; }
static inline void debug_set_modules(unsigned module) { (void)module; }
static inline unsigned debug_get_modules(void) { return 0; }
static inline void debug_enable_color(bool enable) { (void)enable; }
static inline bool debug_is_color_enabled(void) { return false; }
static inline void debug_set_output(const FILE *fp) { (void)fp; }

#endif /* DEBUG_ENABLE */

#endif /* DEBUG_H */
