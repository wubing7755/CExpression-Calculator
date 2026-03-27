#ifndef  LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/**
 * 初始化日志系统，设置最小输出级别
 */
void logger_init(LogLevel level);

/**
 * 动态调整日志级别
 */
void logger_set_level(LogLevel level);

/**
 * 获取当前日志级别（便于调试/测试）
 */
LogLevel logger_get_level(void);

/**
 * 输出日志。WARNING/ERROR 默认写 stderr，其余写 stdout。
 * 调用方无需手动换行，如果 format 已包含换行则按原样输出。
 */
void logger_log(LogLevel level, const char* format, ...);

#endif
