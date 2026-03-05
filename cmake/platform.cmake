# platform.cmake - Cross-platform detection and configuration

# Platform detection
if(WIN32)
    if(MSVC)
        set(PLATFORM_WINDOWS_MSVC ON)
        message(STATUS "[PLATFORM] Windows (MSVC) detected")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        if(MINGW)
            set(PLATFORM_WINDOWS_MINGW ON)
        elseif(MSYS)
            set(PLATFORM_WINDOWS_MSYS2 ON)
        endif()
    endif()
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX ON)
    message(STATUS "[PLATFORM] Linux detected")
elseif(APPLE)
    set(PLATFORM_MACOS ON)
    message(STATUS "[PLATFORM] macOS detected")
endif()

# Compiler detection
if(MSVC)
    set(COMPILER_MSVC ON)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_GCC ON)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_CLANG ON)
endif()

# Architecture detection
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_64BIT ON)
else()
    set(PLATFORM_32BIT ON)
endif()

# Define platform macros
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
endif()

if(PLATFORM_LINUX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

if(PLATFORM_MACOS)
    add_compile_definitions(PLATFORM_MACOS)
endif()

message(STATUS "[PLATFORM] Detection complete: ${CMAKE_SYSTEM_NAME} / ${CMAKE_C_COMPILER_ID}")
