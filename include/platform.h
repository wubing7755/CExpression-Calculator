/**
 * @file platform.h
 * @brief 平台适配层头文件
 * 
 * 提供跨平台的统一接口,隐藏不同平台之间的差异
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/* ========================================================================
 * 平台检测宏 - 由 CMake 自动生成
 * ======================================================================== */

/**
 * @def PLATFORM_WINDOWS
 * @brief Windows 平台标识
 */
#ifdef PLATFORM_WINDOWS
    #define PLATFORM_IS_WINDOWS 1
#else
    #define PLATFORM_IS_WINDOWS 0
#endif

/**
 * @def PLATFORM_LINUX
 * @brief Linux 平台标识
 */
#ifdef PLATFORM_LINUX
    #define PLATFORM_IS_LINUX 1
#else
    #define PLATFORM_IS_LINUX 0
#endif

/**
 * @def PLATFORM_MACOS
 * @brief macOS 平台标识
 */
#ifdef PLATFORM_MACOS
    #define PLATFORM_IS_MACOS 1
#else
    #define PLATFORM_IS_MACOS 0
#endif

/* ========================================================================
 * 跨平台类型定义
 * ======================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================
 * 跨平台函数声明
 * ======================================================================== */

/**
 * @brief 平台初始化
 * @return 0 表示成功,非0表示失败
 */
int platform_init(void);

/**
 * @brief 平台清理
 */
void platform_cleanup(void);

/**
 * @brief 获取平台名称
 * @return 平台名称字符串
 */
const char* platform_get_name(void);

/**
 * @brief 获取当前时间戳(毫秒)
 * @return 时间戳
 */
uint64_t platform_get_tick_ms(void);

/**
 * @brief 睡眠指定毫秒数
 * @param ms 毫秒数
 */
void platform_sleep_ms(uint32_t ms);

#endif /* PLATFORM_H */
