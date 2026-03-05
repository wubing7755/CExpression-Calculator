/**
 * @file platform_win.c
 * @brief Windows 平台特定实现
 */

#include "platform.h"
#include <windows.h>
#include <stdio.h>

/* ========================================================================
 * 公共函数实现
 * ======================================================================== */

int platform_init(void)
{
    /* Windows 平台初始化 - 当前无需特殊处理 */
    return 0;
}

void platform_cleanup(void)
{
    /* Windows 平台清理 - 当前无需特殊处理 */
}

const char* platform_get_name(void)
{
    return "Windows";
}

uint64_t platform_get_tick_ms(void)
{
    /* 使用 Windows Performance Counter 获取高精度时间 */
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;
    
    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    
    QueryPerformanceCounter(&counter);
    
    /* 转换为毫秒 */
    return (counter.QuadPart * 1000) / frequency.QuadPart;
}

void platform_sleep_ms(uint32_t ms)
{
    Sleep(ms);
}
