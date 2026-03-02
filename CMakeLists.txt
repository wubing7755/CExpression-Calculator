# CMakeLists.txt - C语言计算器项目构建配置
# 
# 这是一个使用CMake管理的C语言练习项目
# 
# 使用方法：
#   1. mkdir build && cd build
#   2. cmake -G "MinGW Makefiles" ..
#   3. cmake --build .
#   4. ./calculator.exe

# 如果之前有build目录，先删除它再重新配置！

# CMakeLists.txt - C语言计算器项目构建配置
cmake_minimum_required(VERSION 3.10)

project(CExpressionCalculator VERSION 1.0 LANGUAGES C)

# 设置C语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# ===== .c 源文件 =====
set(SOURCES
    main.c
    calculator.c
)

# 创建可执行文件
add_executable(calculator ${SOURCES})

# ===== 头文件目录 =====
# 头文件在同一目录，需要指定include目录
target_include_directories(calculator PRIVATE ${CMAKE_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    target_link_libraries(calculator m)
endif()

# 打印配置信息
message(STATUS "=== CExpressionCalculator Build Config ===")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Calculator: ${CMAKE_CURRENT_BINARY_DIR}/calculator${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "==========================================")