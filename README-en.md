# CalcExpr

A mathematical expression calculator written in C language, supporting basic operations and parentheses priority.

[![C Version](https://img.shields.io/badge/C-C11-blue)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![CN](https://img.shields.io/badge/中文-red)](README.md)
[![ES](https://img.shields.io/badge/Español-yellow)](README-es.md)

🌐 **Choose Language**: [中文](README.md) | [Español](README-es.md) | **English**

## 📚 Project Overview

This is a practice project for learning C language, implementing mathematical expression evaluation using the **Recursive Descent Parser** algorithm. Through this project, you can learn:

- C language fundamentals
- Recursive descent algorithm
- Lexical analysis basics
- Modular code design

## ✨ Features

- ✅ Addition (+), Subtraction (-), Multiplication (*), Division (/)
- ✅ Parentheses priority handling
- ✅ Operator precedence (multiplication/division > addition/subtraction)
- ✅ Decimal number support
- ✅ Command-line interactive interface
- ✅ Debug system (--debug flag to view parsing process)

## 🛠️ Tech Stack

- **Programming Language**: C (C99)
- **Build Tools**: CMake / GCC / MinGW
- **Algorithm**: Recursive Descent Parser
- **Debug**: Runtime control via (--debug)

## 📁 File Structure

```
CalcExpr/
├── src/                    # Source code
│   ├── main.c            # Main program entry
│   ├── calculator.c       # Calculator public interface
│   ├── parser.c          # Recursive descent parser
│   ├── lexer.c           # Lexer
│   ├── command.c         # Command handler
│   ├── logger.c          # Logging system
│   ├── debug.c            # Debug system
│   └── platform/          # Platform-specific code
├── include/               # Header files
│   ├── calculator.h      # Public interface
│   ├── parser.h         # Parser interface
│   ├── lexer.h          # Lexer interface
│   ├── command.h        # Command interface
│   ├── logger.h         # Logger interface
│   ├── debug.h          # Debug macro interface
│   └── parser_debug.h    # Parser debug macros
├── test/                  # Unit tests
├── CMakeLists.txt        # CMake build configuration
├── build_debug.bat       # Windows debug build script
└── README.md             # Project documentation
```

## 🚀 Quick Start

### Build Project (CMake)

```bash
# 1. Create build directory
mkdir build && cd build

# 2. Configure based on your platform
#    +----------+------------------+----------------------------------+
#    | Platform | Compiler         | Build Command                   |
#    +----------+------------------+----------------------------------+
#    | Windows  | MinGW / MSYS2   | cmake -G "MinGW Makefiles" ..  |
#    |          |                  | cmake --build .                  |
#    +----------+------------------+----------------------------------+
#    | Windows  | MSVC (VS 2022)  | cmake -G "Visual Studio 17 2022"|
#    |          |                  | cmake --build . --config Release |
#    +----------+------------------+----------------------------------+
#    | Linux    | GCC / Clang     | cmake ..                        |
#    |          |                  | cmake --build .                  |
#    +----------+------------------+----------------------------------+
#    | macOS    | Apple Clang     | cmake ..                        |
#    |          |                  | cmake --build .                  |
#    +----------+------------------+----------------------------------+

# Windows MinGW example:
cmake -G "MinGW Makefiles" ..

# Windows MSVC example:
cmake -G "Visual Studio 17 2022" ..

# Linux/macOS example:
cmake ..

# 3. Build
cmake --build .
```

### Run the Program

```bash
# Windows
./bin/calculator.exe

# Linux / macOS
./bin/calculator
```

### Direct GCC Compilation (Not Recommended)

```bash
# Windows
gcc src/main.c src/calculator.c -o calculator.exe -lm

# Linux / macOS
gcc src/main.c src/calculator.c -o calculator -lm
```

### Usage Example

```
Please enter expression> 2+3*4
Expression: 2+3*4
Result:    14.00

Please enter expression> (2+3)*4
Expression: (2+3)*4
Result:    20.00

Please enter expression> (3-5)*6
Expression: (3-5)*6
Result:    -12.00
```

## 📖 Learning Topics

### 1. Recursive Descent Algorithm

Recursive definition of mathematical expressions:

```
Expression = Term { (+|-) Term }
Term       = Factor { (*|/) Factor }
Factor     = Number | '(' Expression ')'
```

This recursive definition naturally supports operator precedence!

### 2. Lexical Analysis (Lexer)

Breaking input string into Tokens:
- Numbers: `123`, `45.67`, `1e-3`
- Operators: `+`, `-`, `*`, `/`
- Parentheses: `(`, `)`
- End marker: `END`

### 3. Modular Design

- `lexer.h/c` - Lexer
- `parser.h/c` - Recursive descent parser
- `calculator.h/c` - Calculator public interface
- `command.h/c` - Command handler
- `logger.h/c` - Unified logging system
- `debug.h/c` - Debug macro system

### 4. Debug System

The project provides a unified debug system, controllable at runtime via command-line arguments, for learning how the recursive descent parser executes.

#### Compile-time Enable

Build with `-DENABLE_DEBUG=ON`:

```bash
# Windows
cmake -G "MinGW Makefiles" -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build

# Or use the provided script
build_debug.bat
```

#### Command-line Arguments

| Argument | Description |
|----------|-------------|
| `--debug` | Enable debug mode |
| `--debug-level=N` | Set debug level (0-5) |
| `--debug-module=NAME` | Set module (lexer/parser/calc/main/all) |

#### Debug Levels

| Level | Value | Description |
|-------|-------|-------------|
| `DEBUG_LEVEL_NONE` | 0 | Disable all debug output |
| `DEBUG_LEVEL_ERROR` | 1 | Error messages only |
| `DEBUG_LEVEL_WARN` | 2 | Warnings and above |
| `DEBUG_LEVEL_INFO` | 3 | Info and above |
| `DEBUG_LEVEL_DEBUG` | 4 | Debug and above (Lexer Token output) |
| `DEBUG_LEVEL_TRACE` | 5 | Trace mode (Parser function call tree + intermediate results) |

#### Debug Output Example

Input `2+3*4` with `--debug --debug-level=5`:

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
Expression: 2+3*4
Result:   14
```

Output explanation:
- `[LEXER]` line shows all Tokens (type@position)
- `[PARSER]` lines show parser function call tree and intermediate results
- Indentation represents recursion depth

## 🤝 Contributing

1. Fork this repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

##  License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

⭐ If this project helps you, please star to show your support!
