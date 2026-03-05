# CMakeLists.txt - C语言计算器项目构建配置
#
# 使用方法：
#   1. mkdir build && cd build
#   2. cmake -G "MinGW Makefiles" ..
#   3. cmake --build . --config Release
#   4. 运行生成的 calculator.exe

# ============================================================
# CMake 最低版本
# ============================================================
cmake_minimum_required(VERSION 3.15)

# ============================================================
# 项目名称、版本号和语言
# ============================================================
project(CExpressionCalculator 
        VERSION 1.0 
        DESCRIPTION "C Expression Calculator"
        LANGUAGES C)

# ============================================================
# 导出 compile_commands.json（VSCode IntelliSense 使用）
# ============================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# 设置默认构建类型（仅对单配置生成器有效，如 MinGW）
# ============================================================
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ============================================================
# 设置 C 语言标准
# ============================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ============================================================
# 源文件列表
# ============================================================
set(SOURCES
    main.c
    calculator.c
)

# ============================================================
# 创建可执行文件
# ============================================================
add_executable(calculator ${SOURCES})

# ============================================================
# 头文件目录
# ============================================================
target_include_directories(calculator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============================================================
# 编译选项
# ============================================================
if(MSVC)
    target_compile_options(calculator PRIVATE
        /W4             # 高警告级别
        /permissive-    # 严格标准模式
    )
else()
    target_compile_options(calculator PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
    )
endif()

# ============================================================
# Debug 编译选项（用于调试）
# ============================================================
target_compile_options(calculator PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
)

# ============================================================
# 输出目录
# ============================================================
set_target_properties(calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================
# 打印构建信息
# ============================================================
message(STATUS "")
message(STATUS "=== CExpressionCalculator Build Config ===")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Binary dir: ${CMAKE_BINARY_DIR}")
message(STATUS "==========================================")
message(STATUS "")