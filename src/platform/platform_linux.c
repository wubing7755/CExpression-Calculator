/**
 * @file platform_linux.c
 * @brief Linux 平台特定实现
 */

#include "platform.h"
#include <unistd.h>
#include <time.h>
#include <stdio.h>

/* ========================================================================
 * 公共函数实现
 * ======================================================================== */

int platform_init(void)
{
    /* Linux 平台初始化 - 当前无需特殊处理 */
    return 0;
}

void platform_cleanup(void)
{
    /* Linux 平台清理 - 当前无需特殊处理 */
}

const char* platform_get_name(void)
{
    return "Linux";
}

uint64_t platform_get_tick_ms(void)
{
    struct timespec ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    /* 转换为毫秒 */
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void platform_sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);  /* usleep 接受微秒 */
}
