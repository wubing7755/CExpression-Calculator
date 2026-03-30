#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"

/* -------------------------------------------------------------------------
 * 配置与状态
 * ------------------------------------------------------------------------- */

static LogLevel g_log_level = LOG_INFO;

/* WARNING/ERROR 走 stderr，其余走 stdout，避免信息混流 */
static FILE* logger_stream_for_level(LogLevel level) {
    return (level >= LOG_WARNING) ? stderr : stdout;
}

/* -------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------- */

void logger_init(LogLevel level) {
    g_log_level = level;
}

void logger_set_level(LogLevel level) {
    g_log_level = level;
}

LogLevel logger_get_level(void) {
    return g_log_level;
}

void logger_log(LogLevel level, const char* format, ...) {
    size_t format_len;

    if (level < g_log_level) {
        return;
    }
    if (format == NULL) {
        return;
    }

    const char* level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG";   break;
        case LOG_INFO:    level_str = "INFO";    break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR";   break;
        default:          level_str = "UNKNOWN"; break;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    FILE* out = logger_stream_for_level(level);

    va_list args;
    va_start(args, format);
    fprintf(out, "[%s] [%s] ", time_str, level_str);
    vfprintf(out, format, args);
    va_end(args);

    format_len = strlen(format);

    /* 错误日志和未换行输出立即刷新，避免提示符滞后。 */
    if (level >= LOG_WARNING || format_len == 0 || format[format_len - 1] != '\n') {
        fflush(out);
    }
}
