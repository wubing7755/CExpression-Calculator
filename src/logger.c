/**
 * @file logger.c
 * @brief 日志模块实现
 * 
 * ## 模块职责
 * 
 * 这个模块提供统一的日志输出接口，具有以下特点：
 *   1. 支持多个日志级别（DEBUG、INFO、WARNING、ERROR）
 *   2. 自动添加时间戳
 *   3. 按级别分流输出（WARNING/ERROR 到 stderr，其他到 stdout）
 *  4. 支持可变参数（类似 printf）
 * 
 * ## 为什么日志要分流？
 * 
 * 将错误和警告输出到 stderr，而正常信息输出到 stdout，有以下好处：
 *   - 可以单独重定向错误信息到文件：./program 2> errors.log
 *   - 交互式程序中，错误信息不会干扰正常输出
 *   - 便于后续日志分析和处理
 */

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"

/* ==========================================================================
 * 配置与状态
 * ========================================================================== */

/**
 * @brief 全局日志级别
 * 
 * 只有级别 >= g_log_level 的日志才会被输出。
 * 例如：设置为 LOG_INFO 时，DEBUG 级别的日志不会输出。
 * 
 * ## 级别数值
 *   DEBUG   = 0  （最低，最详细）
 *   INFO    = 1
 *   WARNING = 2
 *   ERROR   = 3  （最高，最重要）
 * 
 * 数值越大表示越重要。低级别的数值必须小于高级别才能正确过滤。
 */
static LogLevel g_log_level = LOG_INFO;

/* ==========================================================================
 * 内部辅助函数
 * ========================================================================== */

/**
 * @brief 根据日志级别获取输出流
 * 
 * 将不同级别的日志输出到不同的流：
 *   - WARNING 和 ERROR → stderr（错误流）
 *   - DEBUG 和 INFO   → stdout（标准输出）
 * 
 * ## 为什么这样设计？
 * 
 * 在 Unix/Linux 哲学中：
 *   - stdout 用于正常的程序输出
 *   - stderr 用于错误、警告、调试信息
 * 
 * 这样可以通过 shell 重定向分离它们：
 *   $ program > output.txt      # 只保存正常输出
 *   $ program 2> errors.txt     # 只保存错误信息
 *   $ program > out.txt 2>&1    # 保存所有输出
 * 
 * @param level 日志级别
 * @return 应该输出到的 FILE 流
 */
static FILE* logger_stream_for_level(LogLevel level) {
    /* 使用条件表达式：WARNING 及以上返回 stderr，否则返回 stdout */
    return (level >= LOG_WARNING) ? stderr : stdout;
}

/* ==========================================================================
 * 公共 API
 * ========================================================================== */

/**
 * @brief 初始化日志系统
 * 
 * 设置最小日志级别。只有级别 >= 此值的日志才会被输出。
 * 
 * @param level 最小日志级别
 * 
 * ## 使用示例
 * 
 * @code
 *   logger_init(LOG_DEBUG);  // 输出所有日志
 *   logger_init(LOG_INFO);   // 只输出 INFO、WARNING、ERROR
 *   logger_init(LOG_ERROR);  // 只输出 ERROR
 * @endcode
 */
void logger_init(LogLevel level) {
    g_log_level = level;
}

/**
 * @brief 动态调整日志级别
 * 
 * 可以在程序运行时改变日志级别。
 * 适用于需要动态开启/关闭调试输出的场景。
 * 
 * @param level 新的日志级别
 */
void logger_set_level(LogLevel level) {
    g_log_level = level;
}

/**
 * @brief 获取当前日志级别
 * 
 * 便于调试和测试代码获取当前的日志配置。
 * 
 * @return 当前日志级别
 */
LogLevel logger_get_level(void) {
    return g_log_level;
}

/**
 * @brief 输出日志
 * 
 * 这是日志模块的核心函数，提供类似 printf 的接口。
 * 
 * ## 功能说明
 * 
 *   1. 级别过滤：检查 g_log_level，决定是否输出
 *   2. 添加时间戳：[YYYY-MM-DD HH:MM:SS]
 *   3. 添加级别标签：[DEBUG]、[INFO]、[WARNING]、[ERROR]
 *   4. 输出消息：使用 vfprintf 输出格式化内容
 *   5. 自动刷新：确保输出立即显示（在某些情况下）
 * 
 * ## 时间戳格式
 * 
 * 使用 strftime 格式化为 "YYYY-MM-DD HH:MM:SS" 格式，
 * 例如 "2026-03-30 22:30:00"。
 * 
 * ## 刷新策略
 * 
 * 在以下情况会立即刷新输出缓冲区：
 *   - WARNING 或 ERROR 级别（确保错误信息不丢失）
 *   - 格式字符串不以换行符结尾（提示信息等中间输出）
 * 
 * 这样可以确保：
 *   - 用户立即看到错误信息
 *   - 不会出现提示符覆盖未换行输出的问题
 * 
 * @param level 日志级别
 * @param format printf 风格的格式化字符串
 * @param ... 可变参数
 */
void logger_log(LogLevel level, const char* format, ...) {
    size_t format_len;

    /* ---------------------------------------------------------------
     * 级别过滤
     * --------------------------------------------------------------- */
    if (level < g_log_level) {
        return;  /* 级别太低，不输出 */
    }
    if (format == NULL) {
        return;  /* 空格式字符串，忽略 */
    }

    /* ---------------------------------------------------------------
     * 级别标签转换
     * ---------------------------------------------------------------
     * 
     * 将 LogLevel 枚举转换为可读字符串。
     * 使用 switch 确保编译时检查所有情况。
     */
    const char* level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG";   break;
        case LOG_INFO:    level_str = "INFO";    break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR";   break;
        default:          level_str = "UNKNOWN"; break;
    }

    /* ---------------------------------------------------------------
     * 生成时间戳
     * --------------------------------------------------------------- */
    time_t now = time(NULL);              /* 获取当前时间 */
    struct tm* t = localtime(&now);       /* 转换为本地时间结构 */
    char time_str[20];                     /* 足够存储 "YYYY-MM-DD HH:MM:SS\0" */
    
    /* 格式化时间：%Y=4位年，%m=月，%d=日，%H=24小时，%M=分，%S=秒 */
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    /* ---------------------------------------------------------------
     * 选择输出流
     * --------------------------------------------------------------- */
    FILE* out = logger_stream_for_level(level);

    /* ---------------------------------------------------------------
     * 输出日志头部和时间戳
     * --------------------------------------------------------------- */
    va_list args;
    va_start(args, format);
    
    /* 输出格式：[时间戳] [级别] 消息 */
    fprintf(out, "[%s] [%s] ", time_str, level_str);
    vfprintf(out, format, args);
    va_end(args);

    /* ---------------------------------------------------------------
     * 刷新策略
     * ---------------------------------------------------------------
     * 
     * 刷新条件：
     *   1. 级别 >= WARNING：确保错误信息不丢失
     *   2. 格式为空
     *   3. 格式不以换行符结尾
     * 
     * 这种设计确保交互式程序的输出体验：
     *   - 用户输入提示不会与输出混在一起
     *   - 错误信息立即显示
     */
    format_len = strlen(format);

    if (level >= LOG_WARNING || format_len == 0 || format[format_len - 1] != '\n') {
        fflush(out);  /* 立即刷新缓冲区 */
    }
}
