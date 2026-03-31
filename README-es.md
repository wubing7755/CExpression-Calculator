# CalcExpr

Una calculadora de expresiones matemáticas escrita en lenguaje C, que soporta operaciones básicas y prioridad de paréntesis.

[![C Version](https://img.shields.io/badge/C-C11-blue)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![CN](https://img.shields.io/badge/中文-red)](README.md)
[![EN](https://img.shields.io/badge/English-blue)](README-en.md)

🌐 **Elegir Idioma**: [English](README-en.md) | [中文](README.md) | **Español**

## 📚 Descripción del Proyecto

Este es un proyecto de práctica para aprender el lenguaje C, implementando la evaluación de expresiones matemáticas usando el algoritmo **Recursive Descent Parser** (Analizador Recursivo Descendente). A través de este proyecto, puedes aprender:

- Fundamentos del lenguaje C
- Algoritmo recursivo descendente
- Fundamentos del análisis léxico
- Diseño de código modular

## ✨ Características

- ✅ Suma (+), Resta (-), Multiplicación (*), División (/)
- ✅ Manejo de prioridad de paréntesis
- ✅ Precedencia de operadores (multiplicación/división > suma/resta)
- ✅ Soporte para números decimales
- ✅ Interfaz interactiva de línea de comandos
- ✅ Sistema de depuración (--debug para ver el proceso de análisis)

## 🛠️ Tecnologías

- **Lenguaje de Programación**: C (C99)
- **Herramientas de Construcción**: CMake / GCC / MinGW
- **Algoritmo**: Analizador Recursivo Descendente
- **Depuración**: Control en tiempo de ejecución (--debug)

## 📁 Estructura de Archivos

```
CalcExpr/
├── src/                    # Código fuente
│   ├── main.c            # Punto de entrada principal
│   ├── calculator.c       # Interfaz pública de la calculadora
│   ├── parser.c          # Analizador recursivo descendente
│   ├── lexer.c           # Analizador léxico
│   ├── command.c         # Manejador de comandos
│   ├── logger.c          # Sistema de registro
│   ├── debug.c            # Sistema de depuración
│   └── platform/          # Código específico de plataforma
├── include/               # Archivos de encabezado
│   ├── calculator.h      # Interfaz pública
│   ├── parser.h         # Interfaz del analizador
│   ├── lexer.h          # Interfaz del analizador léxico
│   ├── command.h        # Interfaz de comandos
│   ├── logger.h         # Interfaz del registrador
│   ├── debug.h          # Interfaz de macros de depuración
│   └── parser_debug.h    # Macros de depuración del analizador
├── test/                  # Pruebas unitarias
├── CMakeLists.txt        # Configuración de compilación CMake
├── build_debug.bat       # Script de compilación de depuración para Windows
└── README.md             # Documentación del proyecto
```

## 🚀 Inicio Rápido

### Compilar el Proyecto (CMake)

```bash
# 1. Crear directorio de compilación
mkdir build && cd build

# 2. Configurar según tu plataforma
#    +----------+------------------+----------------------------------+
#    | Plataforma| Compilador      | Comando de compilación          |
#    +----------+------------------+----------------------------------+
#    | Windows  | MinGW / MSYS2  | cmake -G "MinGW Makefiles" .. |
#    |          |                 | cmake --build .                  |
#    +----------+------------------+----------------------------------+
#    | Windows  | MSVC (VS 2022) | cmake -G "Visual Studio 17 2022"|
#    |          |                 | cmake --build . --config Release |
#    +----------+------------------+----------------------------------+
#    | Linux    | GCC / Clang     | cmake ..                        |
#    |          |                 | cmake --build .                  |
#    +----------+------------------+----------------------------------+
#    | macOS    | Apple Clang     | cmake ..                        |
#    |          |                 | cmake --build .                  |
#    +----------+------------------+----------------------------------+

# Ejemplo Windows MinGW:
cmake -G "MinGW Makefiles" ..

# Ejemplo Windows MSVC:
cmake -G "Visual Studio 17 2022" ..

# Ejemplo Linux/macOS:
cmake ..

# 3. Compilar
cmake --build .
```

### Ejecutar el Programa

```bash
# Windows
./bin/calculator.exe

# Linux / macOS
./bin/calculator
```

### Compilación directa con GCC (No recomendado)

```bash
# Windows
gcc src/main.c src/calculator.c -o calculator.exe -lm

# Linux / macOS
gcc src/main.c src/calculator.c -o calculator -lm
```

### Ejemplo de Uso

```
Por favor ingrese la expresión> 2+3*4
Expresión: 2+3*4
Resultado: 14.00

Por favor ingrese la expresión> (2+3)*4
Expresión: (2+3)*4
Resultado: 20.00

Por favor ingrese la expresión> (3-5)*6
Expresión: (3-5)*6
Resultado: -12.00
```

## 📖 Temas de Aprendizaje

### 1. Algoritmo Recursivo Descendente

Definición recursiva de expresiones matemáticas:

```
Expresión = Término { (+|-) Término }
Término   = Factor { (*|/) Factor }
Factor    = Número | '(' Expresión ')'
```

¡Esta definición recursiva soporta naturalmente la precedencia de operadores!

### 2. Análisis Léxico (Lexer)

Dividir la cadena de entrada en Tokens:
- Números: `123`, `45.67`, `1e-3`
- Operadores: `+`, `-`, `*`, `/`
- Paréntesis: `(`, `)`
- Marcador final: `END`

### 3. Diseño Modular

- `lexer.h/c` - Analizador léxico
- `parser.h/c` - Analizador recursivo descendente
- `calculator.h/c` - Interfaz pública de la calculadora
- `command.h/c` - Manejador de comandos
- `logger.h/c` - Sistema de registro unificado
- `debug.h/c` - Sistema de macros de depuración

### 4. Sistema de Depuración

El proyecto proporciona un sistema de depuración unificado, controlable en tiempo de ejecución mediante argumentos de línea de comandos, para aprender cómo ejecuta el analizador recursivo descendente.

#### Activar en Tiempo de Compilación

Compilar con `-DENABLE_DEBUG=ON`:

```bash
# Windows
cmake -G "MinGW Makefiles" -DENABLE_DEBUG=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build

# O usar el script proporcionado
build_debug.bat
```

#### Argumentos de Línea de Comandos

| Argumento | Descripción |
|-----------|-------------|
| `--debug` | Activar modo de depuración |
| `--debug-level=N` | Establecer nivel de depuración (0-5) |
| `--debug-module=NAME` | Establecer módulo (lexer/parser/calc/main/all) |

#### Niveles de Depuración

| Nivel | Valor | Descripción |
|-------|-------|-------------|
| `DEBUG_LEVEL_NONE` | 0 | Desactivar toda la salida de depuración |
| `DEBUG_LEVEL_ERROR` | 1 | Solo mensajes de error |
| `DEBUG_LEVEL_WARN` | 2 | Advertencias y superior |
| `DEBUG_LEVEL_INFO` | 3 | Información y superior |
| `DEBUG_LEVEL_DEBUG` | 4 | Depuración y superior (Salida de Tokens del Lexer) |
| `DEBUG_LEVEL_TRACE` | 5 | Modo traza (Árbol de llamadas de funciones del Parser + resultados intermedios) |

#### Ejemplo de Salida de Depuración

Entrada `2+3*4` con `--debug --debug-level=5`:

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
Expresión: 2+3*4
Resultado:   14
```

Explicación de la salida:
- Línea `[LEXER]` muestra todos los Tokens (tipo@posición)
- Líneas `[PARSER]` muestran el árbol de llamadas de funciones del analizador y resultados intermedios
- La sangría representa la profundidad de la recursión

## 🤝 Contribuciones

1. Fork este repositorio
2. Crea una rama de característica (`git checkout -b feature/amazing-feature`)
3. Confirma tus cambios (`git commit -m 'Add some amazing feature'`)
4. Envía la rama (`git push origin feature/amazing-feature`)
5. Abre un Pull Request

## Licencia

Este proyecto está licenciado bajo la Licencia MIT - consulta el archivo [LICENSE](LICENSE) para más detalles.

---

⭐ Si este proyecto te ayuda, por favor pon una estrella para mostrar tu apoyo.
