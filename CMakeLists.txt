# =============================================================================
# CalcExpr - CMake 构建配置 (企业级跨平台)
# =============================================================================
#
# 支持的平台和编译器:
#   +----------+------------------+----------------------------------+
#   | 平台     | 编译器           | 构建命令                         |
#   +----------+------------------+----------------------------------+
#   | Windows  | MinGW / MSYS2    | cmake -G "MinGW Makefiles" ..   |
#   |          |                  | cmake --build .                  |
#   +----------+------------------+----------------------------------+
#   | Windows  | MSVC (VS 2022)   | cmake -G "Visual Studio 17 2022"|
#   |          |                  | cmake --build . --config Release |
#   +----------+------------------+----------------------------------+
#   | Linux    | GCC / Clang      | cmake ..                         |
#   |          |                  | cmake --build .                   |
#   +----------+------------------+----------------------------------+
#   | macOS    | Apple Clang      | cmake ..                         |
#   |          |                  | cmake --build .                   |
#   +----------+------------------+----------------------------------+
#
# 快速开始:
#   1. mkdir build && cd build
#   2. cmake -G "MinGW Makefiles" ..   # 根据平台选择生成器
#   3. cmake --build .
#   4. ./bin/calculator
# =============================================================================

cmake_minimum_required(VERSION 3.15)

# =============================================================================
# 项目信息
# =============================================================================
project(CalcExpr 
    VERSION 1.0.0
    DESCRIPTION "A cross-platform expression calculator written in C"
    HOMEPAGE_URL "https://github.com/wubing7755/CalcExpr"
    LANGUAGES C
)

# CTest 开关（默认开启 BUILD_TESTING）
include(CTest)

# =============================================================================
# 导出 compile_commands.json (VSCode IntelliSense)
# =============================================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# 构建类型配置
# =============================================================================
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS 
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# =============================================================================
# C 语言标准
# =============================================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# 调试系统配置
# =============================================================================
option(ENABLE_DEBUG "Enable debug macro system (零开销 when OFF)" OFF)

if(ENABLE_DEBUG)
    message(STATUS "[DEBUG] Debug system ENABLED")
    add_compile_definitions(DEBUG_ENABLE=1)
    add_compile_definitions(DEBUG_DEFAULT_LEVEL=5)  # TRACE level
    add_compile_definitions(DEBUG_DEFAULT_MODULE=0xFF)  # ALL modules
else()
    message(STATUS "[DEBUG] Debug system disabled (zero overhead)")
    add_compile_definitions(DEBUG_ENABLE=0)
endif()

# =============================================================================
# 源文件 - 按模块组织
# =============================================================================
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.c
)

# debug.c 仅在 ENABLE_DEBUG=ON 时编译
if(ENABLE_DEBUG)
    list(APPEND CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/debug.c)
endif()

# 平台特定源文件
set(PLATFORM_SOURCES)
if(WIN32)
    list(APPEND PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/platform_win.c)
elseif(UNIX AND NOT APPLE)
    list(APPEND PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/platform_linux.c)
elseif(APPLE)
    list(APPEND PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/platform_mac.c)
endif()

# 合并所有源文件
set(ALL_SOURCES ${CORE_SOURCES} ${PLATFORM_SOURCES})

# =============================================================================
# 创建可执行文件
# =============================================================================
add_executable(calculator ${ALL_SOURCES})

# 单元测试可执行文件（不包含 main.c）
if(BUILD_TESTING)
    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_calculator.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/command.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.c
    )
    if(ENABLE_DEBUG)
        list(APPEND TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/debug.c)
    endif()
    add_executable(calculator_tests ${TEST_SOURCES})
endif()

# =============================================================================
# 头文件目录
# =============================================================================
target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
)

if(BUILD_TESTING)
    target_include_directories(calculator_tests
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()

# =============================================================================
# 平台检测与配置
# =============================================================================
include(cmake/platform.cmake)

# =============================================================================
# 编译器特定选项
# =============================================================================
if(MSVC)
    message(STATUS "[INFO] Using MSVC compiler")
    target_compile_options(calculator PRIVATE
        /W4             # 警告级别 4
        /permissive-    # 严格标准模式
        /utf-8          # UTF-8 源码
    )
    
    # Release 优化
    target_compile_options(calculator PRIVATE
        $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG>
    )

    if(BUILD_TESTING)
        target_compile_options(calculator_tests PRIVATE
            /W4
            /permissive-
            /utf-8
            $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG>
        )
    endif()
    
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "[INFO] Using GCC/Clang compiler")
    target_compile_options(calculator PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
    )
    
    # Release 优化
    target_compile_options(calculator PRIVATE
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
    
    # Debug 信息
    target_compile_options(calculator PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
    )

    if(BUILD_TESTING)
        target_compile_options(calculator_tests PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            $<$<CONFIG:Release>:-O2 -DNDEBUG>
            $<$<CONFIG:Debug>:-g -O0>
        )
    endif()
endif()

# =============================================================================
# 链接库
# =============================================================================
# 数学库 - Unix-like 系统需要, Windows 不需要
if(UNIX AND NOT APPLE)
    target_link_libraries(calculator PRIVATE m)
    if(BUILD_TESTING)
        target_link_libraries(calculator_tests PRIVATE m)
    endif()
elseif(APPLE)
    target_link_libraries(calculator PRIVATE m)
    if(BUILD_TESTING)
        target_link_libraries(calculator_tests PRIVATE m)
    endif()
endif()

# =============================================================================
# 输出目录配置
# =============================================================================
set_target_properties(calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "calculator"
)

if(BUILD_TESTING)
    set_target_properties(calculator_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        OUTPUT_NAME "test_calculator"
    )
endif()

# =============================================================================
# 安装规则
# =============================================================================
install(TARGETS calculator
    RUNTIME DESTINATION bin
)

# =============================================================================
# CTest 注册
# =============================================================================
if(BUILD_TESTING)
    add_test(NAME calc_tests_all COMMAND calculator_tests)
    add_test(NAME calc_tests_success COMMAND calculator_tests --suite=success)
    add_test(NAME calc_tests_error COMMAND calculator_tests --suite=error)
    add_test(NAME calc_tests_api COMMAND calculator_tests --suite=api)
    add_test(NAME calc_tests_error_div0 COMMAND calculator_tests --suite=error --filter=1/0)
endif()

# =============================================================================
# 打印构建信息
# =============================================================================
message(STATUS "")
message(STATUS "============================================================")
message(STATUS "  CalcExpr - Build Configuration")
message(STATUS "============================================================")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Platform:       ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler:       ${CMAKE_C_COMPILER_ID} (${CMAKE_C_COMPILER})")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Standard:     C${CMAKE_C_STANDARD}")
message(STATUS "  Source Dir:     ${CMAKE_SOURCE_DIR}")
message(STATUS "  Build Dir:      ${CMAKE_BINARY_DIR}")
message(STATUS "============================================================")
message(STATUS "")
