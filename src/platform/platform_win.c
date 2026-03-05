/**
 * @file platform_win.c
 * @brief Windows 平台特定实现
 */

#include "platform.h"
#include <windows.h>
#include <io.h>
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

void platform_enable_utf8(void)
{
    // 设置控制台代码页为 UTF-8 (65001)
    // 先判断是否在控制台中运行
    if (_isatty(_fileno(stdout)))
    {
        UINT output_cp = GetConsoleOutputCP();
        UINT input_cp  = GetConsoleCP();

        // 只有当不是 UTF-8 时才修改
        if (output_cp != 65001)
        {
            SetConsoleOutputCP(65001);
        }

        if (input_cp != 65001)
        {
            SetConsoleCP(65001);
        }
    }
}
