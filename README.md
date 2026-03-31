# CalcExpr

一个用 C 语言编写的表达式计算器，支持基本数学运算和括号优先级。

[![C Version](https://img.shields.io/badge/C-C11-blue)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![EN](https://img.shields.io/badge/English-blue)](README-en.md)
[![ES](https://img.shields.io/badge/Español-yellow)](README-es.md)

🌐 **选择语言**: [English](README-en.md) | [Español](README-es.md) | **中文**

## 📚 项目简介

这是一个用于学习 C 语言的练习项目，采用**递归下降解析器（Recursive Descent Parser）**算法实现数学表达式求值。通过这个项目，你可以学习到：

- C 语言基础编程
- 递归下降算法
- 词法分析基础
- 模块化代码设计

## ✨ 功能特性

- ✅ 加法 (+)、减法 (-)、乘法 (*)、除法 (/)
- ✅ 括号优先级处理
- ✅ 运算符优先级（乘除 > 加减）
- ✅ 支持小数
- ✅ 命令行交互界面
- ✅ 调试系统（支持 --debug 参数查看解析过程）

## 🛠️ 技术栈

- **编程语言**: C (C99)
- **构建工具**: CMake / GCC / MinGW
- **算法**: 递归下降解析器
- **调试**: 运行时参数控制 (--debug)

## 📁 文件结构

```
CalcExpr/
├── src/                    # 源代码
│   ├── main.c            # 主程序入口
│   ├── calculator.c       # 计算器公共接口
│   ├── parser.c          # 递归下降解析器
│   ├── lexer.c           # 词法分析器
│   ├── command.c         # 命令行命令处理
│   ├── logger.c          # 日志系统
│   ├── debug.c           # 调试系统
│   └── platform/          # 平台相关代码
├── include/               # 头文件
│   ├── calculator.h      # 公共接口
│   ├── parser.h         # 解析器接口
│   ├── lexer.h          # 词法分析器接口
│   ├── command.h        # 命令接口
│   ├── logger.h         # 日志接口
│   ├── debug.h          # 调试宏接口
│   └── parser_debug.h    # 解析器调试宏
├── test/                  # 单元测试
├── CMakeLists.txt        # CMake 构建配置
├── build_debug.bat       # Windows 调试构建脚本
└── README.md             # 项目说明
```

## 🚀 快速开始

### 编译项目 (CMake)

```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 根据平台选择生成器配置项目
#    +----------+------------------+----------------------------------+
#    | 平台     | 编译器           | 构建命令                         |
#    +----------+------------------+----------------------------------+
#    | Windows  | MinGW / MSYS2    | cmake -G "MinGW Makefiles" ..   |
#    |          |                  | cmake --build .                  |
#    +----------+------------------+----------------------------------+
#    | Windows  | MSVC (VS 2022)   | cmake -G "Visual Studio 17 2022"|
#    |          |                  | cmake --build . --config Release |
#    +----------+------------------+----------------------------------+
#    | Linux    | GCC / Clang      | cmake ..                         |
#    |          |                  | cmake --build .                   |
#    +----------+------------------+----------------------------------+
#    | macOS    | Apple Clang      | cmake ..                         |
#    |          |                  | cmake --build .                   |
#    +----------+------------------+----------------------------------+

# Windows MinGW 示例:
cmake -G "MinGW Makefiles" ..

# Windows MSVC 示例:
cmake -G "Visual Studio 17 2022" ..

# Linux/macOS 示例:
cmake ..

# 3. 编译
cmake --build .
```

### 运行程序

```bash
# Windows
./bin/calculator.exe

# Linux / macOS
./bin/calculator
```

## 🧪 测试

项目已集成 CTest，并支持在测试程序中按套件/关键字过滤测试。

### 运行全部测试

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### 运行指定测试套件

```bash
# 仅运行错误用例套件
ctest --test-dir build -R calc_tests_error --output-on-failure
```

可用测试名：
- `calc_tests_all`
- `calc_tests_success`
- `calc_tests_error`
- `calc_tests_api`
- `calc_tests_error_div0`（示例：带过滤参数）

### 直接运行测试程序并过滤

```bash
# 查看支持的参数与用例
./build/bin/test_calculator --help
./build/bin/test_calculator --list

# 仅跑成功用例
./build/bin/test_calculator --suite=success

# 仅跑错误用例中包含 1/0 的测试
./build/bin/test_calculator --suite=error --filter=1/0
```

### 使用示例

```
请输入表达式> 2+3*4
表达式: 2+3*4
结果:   14.00

请输入表达式> (2+3)*4
表达式: (2+3)*4
结果:   20.00

请输入表达式> (3-5)*6
表达式: (3-5)*6
结果:   -12.00
```

## 📖 学习内容

### 1. 递归下降算法

数学表达式的递归定义：

```
表达式 = 项 { (+|-) 项 }
项     = 因子 { (*|/) 因子 }
因子   = 数字 | '(' 表达式 ')'
```

这种递归定义天然支持运算符优先级！

### 2. 词法分析 (Lexer)

将输入字符串分解为 Token（标记）：
- 数字: `123`, `45.67`, `1e-3`
- 运算符: `+`, `-`, `*`, `/`
- 括号: `(`, `)`
- 结束标记: `END`

### 3. 模块化设计

- `lexer.h/c` - 词法分析器
- `parser.h/c` - 递归下降解析器
- `calculator.h/c` - 计算器公共接口
- `command.h/c` - 命令行命令处理
- `logger.h/c` - 统一日志系统
- `debug.h/c` - 调试宏系统

### 4. 调试系统

项目提供了统一的调试系统，支持在运行时通过命令行参数开启，用于学习递归下降解析器的执行过程。

#### 编译时启用

构建时需添加 `-DENABLE_DEBUG=ON`：

```bash
# Windows
cmake -G "MinGW Makefiles" -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build

# 或使用项目提供的构建脚本
build_debug.bat
```

#### 命令行参数

| 参数 | 说明 |
|------|------|
| `--debug` | 开启调试模式 |
| `--debug-level=N` | 设置调试级别 (0-5) |
| `--debug-module=NAME` | 设置模块 (lexer/parser/calc/main/all) |

#### 调试级别

| 级别 | 值 | 说明 |
|------|------|------|
| `DEBUG_LEVEL_NONE` | 0 | 关闭所有调试输出 |
| `DEBUG_LEVEL_ERROR` | 1 | 仅错误信息 |
| `DEBUG_LEVEL_WARN` | 2 | 警告及以上 |
| `DEBUG_LEVEL_INFO` | 3 | 信息及以上 |
| `DEBUG_LEVEL_DEBUG` | 4 | 调试及以上（Lexer Token 输出） |
| `DEBUG_LEVEL_TRACE` | 5 | 跟踪模式（Parser 函数调用树 + 中间计算结果） |

#### 调试输出示例

输入 `2+3*4` with `--debug --debug-level=5`：

```
[LEXER] NUM@0 | PLUS@1 | NUM@2 | MUL@3 | NUM@4 | END@5
[PARSER] │   parseExpression()
[PARSER] │   parseTerm()
[PARSER] │   parseUnary()
[PARSER] │   parsePrimary()
[PARSER] │   parseTerm()
[PARSER] │   parseUnary()
[PARSER] │   parsePrimary()
[PARSER] │   parseUnary()
[PARSER] │   parsePrimary()
[PARSER] 3 * 4 = 12
[PARSER] 2 + 12 = 14
表达式: 2+3*4
结果:   14
```

输出说明：
- `[LEXER]` 行显示所有 Token（类型@位置）
- `[PARSER]` 行显示解析器函数调用树和中间计算结果
- 缩进表示递归深度

## 🤝 如何贡献

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 开启 Pull Request

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

---

⭐ 如果这个项目对你有帮助，请 star 支持！
