#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

static LogLevel g_log_level = LOG_INFO;

void logger_init(LogLevel level) {
    g_log_level = level;
}

void logger_set_level(LogLevel level) {
    g_log_level = level;
}

void logger_log(LogLevel level, const char* format, ...) {
    if (level < g_log_level) {
        return;
    }

    const char* level_str;

    switch (level) {
        case LOG_DEBUG: 
            level_str = "DEBUG"; 
            break;
        case LOG_INFO: 
            level_str = "INFO"; 
            break;
        case LOG_WARNING: 
            level_str = "WARNING"; 
            break;
        case LOG_ERROR: 
            level_str = "ERROR"; 
            break;
        default: 
            level_str = "UNKNOWN"; 
            break;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    va_list args;
    va_start(args, format);
    fprintf(stdout, "[%s] [%s] ", time_str, level_str);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}
