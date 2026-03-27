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

## 🛠️ 技术栈

- **编程语言**: C (C11)
- **构建工具**: CMake / GCC
- **算法**: 递归下降解析器

## 📁 文件结构

```
CalcExpr/
├── main.c           # 主程序入口
├── calculator.c     # 计算器核心逻辑（待实现）
├── calculator.h     # 计算器头文件
├── token.h          # Token 类型定义
├── CMakeLists.txt   # CMake 构建配置
└── README.md        # 项目说明
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

### 直接使用 GCC 编译 (不推荐)

```bash
# Windows
gcc src/main.c src/calculator.c -o calculator.exe -lm

# Linux / macOS
gcc src/main.c src/calculator.c -o calculator -lm
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
- 数字: `123`, `45.67`
- 运算符: `+`, `-`, `*`, `/`
- 括号: `(`, `)`

### 3. 模块化设计

- `token.h` - Token 类型定义
- `calculator.c` - 计算逻辑
- `main.c` - 用户交互

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
