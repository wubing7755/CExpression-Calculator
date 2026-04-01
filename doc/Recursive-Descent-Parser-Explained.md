# Recursive Descent Parser Explained

> 📖 This article is also available in Chinese: [递归下降解析器原理详解](./递归下降解析器原理详解.md)

---

## Table of Contents

1. [Why Recursive Descent?](#1-why-recursive-descent)
2. [Why Layered Priority?](#2-why-layered-priority)
3. [Loop vs Recursion for Operator Chains](#3-loop-vs-recursion)
4. [Parentheses Handling](#4-parentheses-handling)
5. [Unary Operator Special Treatment](#5-unary-operators)
6. [Complete Execution Trace](#6-complete-trace)
7. [Key Design Principles](#7-design-principles)
8. [LL(1) Grammar Correspondence](#8-ll1-grammar)
9. [Actual Code Structure](#9-code-structure)
10. [Debugging Tips](#10-debugging)

---

## 1. Why Recursive Descent?

### 1.1 Expressions Can Be Represented as Trees

The expression `9*8-1` can be represented as a **syntax tree**:

```
          (-)
         /   \
        (*)   (1)
       /   \
      (9)   (8)
```

**Tree characteristics:**
- Leaf nodes are numbers
- Internal nodes are operators
- Subtrees are computed first (higher priority operators at lower levels)

### 1.2 Core Idea of Recursive Descent

**Use function calls to build and traverse the tree**

```c
parseExpression()  → creates (-) node
    └─ parseTerm()  → creates (*) node
            ├─ parseUnary() → returns 9
            └─ parseUnary() → returns 8
```

Each grammar rule corresponds to a function, and functions call each other to "分解" (decompose) the expression.

---

## 2. Why Layered Priority?

### 2.1 Operator Precedence Problem

In mathematics, multiplication is calculated before addition:

```
9+8*1  ≠  (9+8)*1
  11    ≠    17
```

### 2.2 How to Implement "Multiplication/Division Before Addition/Subtraction"

**Method: Layered processing, each layer handles one priority level**

```
Expression (handles + -)          ← Lowest priority, calculated last
    └─ Term (handles * /)        ← Medium priority, calculated in middle
            └─ Unary (handles unary -) ← Higher priority
                    └─ Primary (numbers and parentheses) ← Highest priority, calculated first
```

**Execution order (starting from innermost):**

```
1. Primary reads numbers 9, 8, 1
2. Unary handles unary operators (none in this example)
3. Term calculates 9*8=72
4. Expression calculates 72-1=71
```

---

## 3. Loop vs Recursion for Operator Chains

### 3.1 Problem: How to handle `9*8*2`?

**Wrong approach (left recursion, causes infinite recursion):**

```
Expression ::= Expression '*' Term  ← Always starts with Expression, infinite recursion
```

**Correct approach (using loops):**

```
Expression ::= Term { ('+' | '-') Term }
               ↑
               └─ Get left operand first
                              ↑↑↑↑↑↑
                              └─┘ │
                              Loop processes remaining operands
```

### 3.2 Loop Processing Principle

```c
// Pseudocode
double parseExpression() {
    // Step 1: Get first operand
    left = parseTerm();

    // Step 2: Loop to process subsequent operators
    while (current token is + or -) {
        op = current operator;
        parserNextToken();       // Consume operator
        right = parseTerm();     // Get right operand
        left = applyOp(left, op, right);  // Calculate and update left
    }

    return left;
}
```

### 3.3 Trace of `9*8*2`

```
Input: 9*8*2

First loop iteration:
  left = 9
  Encounter *, save op=*, consume *
  right = 8
  left = 9*8 = 72

Second loop iteration:
  left = 72
  Encounter *, save op=*, consume *
  right = 2
  left = 72*2 = 144

Loop ends (encounter END)
Return 144
```

---

## 4. Parentheses Handling

### 4.1 Why Are Parentheses Special?

Because parentheses **change precedence**:

```
(9+1)*2  requires 9+1 to be calculated first
```

### 4.2 How to Handle Parentheses

**Key: When encountering `(`, recursively call the parser itself**

```c
double parsePrimary() {
    if (is number) {
        return number value;
    }

    if (is '(') {
        parserNextToken();              // Consume '('
        value = parseExpression();       // ⚡ Recursion! Restart from Expression
        // At this point, the expression inside parentheses is fully parsed
        parserNextToken();              // Consume ')'
        return value;
    }
}
```

### 4.3 Trace of `(9+1)*2`

```
Input: (9+1)*2

Main flow (Expression):
  └─ Term:
        ├─ Unary:
        │     └─ Primary: encounters '('
        │           ├─ Consume '('
        │           ├─ Recursively call parseExpression()
        │           │
        │           │  ⚡ Recursive layer: parseExpression()
        │           │    ├─ parseTerm() → 9
        │           │    ├─ encounter '+'
        │           │    ├─ parseTerm() → 1
        │           │    └─ return 9+1=10
        │           │
        │           ├─ Consume ')'
        │           └─ return 10
        │
        ├─ encounter '*'
        └─ Unary → Primary: return 2

Main flow continues:
  Calculate 10 * 2 = 20
```

---

## 5. Unary Operator Special Treatment

### 5.1 What Are Unary Operators?

```c
--5    // Two minus signs = positive 5
-3+4   // Minus is a unary operator
+-3    // Plus is also a unary operator
```

### 5.2 Why Not Use Recursion for Unary Operators?

**Wrong design (causes stack overflow):**

```c
Unary ::= ('+' | '-') Unary | Primary
// For input ---5, it would recurse 5 times
```

**Correct design: Use loop counting**

```c
Unary ::= ('+' | '-')* Primary
// Count how many minus signs, negate if odd number
```

### 5.3 Code Implementation

```c
double parseUnary() {
    // Step 1: Count minus signs
    int negative_count = 0;
    while (current is + or -) {
        if (is -) negative_count++;
        parserNextToken();  // Consume operator
    }

    // Step 2: Get operand (only one recursion)
    value = parsePrimary();

    // Step 3: Apply minus sign
    if (negative_count % 2 == 1) {
        value = -value;
    }

    return value;
}
```

### 5.4 Trace of `--5`

```
Input: --5

Step 1: Count minus signs
  negative_count = 2

Step 2: Get operand
  parsePrimary() returns 5

Step 3: Apply minus
  2 % 2 == 0, so don't negate
  return 5
```

---

## 6. Complete Execution Trace

### Input: `(9+1)*2-8/4`

Let's trace step by step:

```
 parseExpression()                                               
   ├─ Call parseTerm()                                          
   │     └─ parseTerm():                                        
   │           ├─ Call parseUnary()                             
   │           │     └─ parseUnary():                            
   │           │           ├─ Minus count: 0                     
   │           │           └─ parsePrimary():                   
   │           │                 ├─ Encounter '('                 
   │           │                 ├─ Recursively call parseExpression() 
   │           │                 │                                
   │           │                 │  ⚡ Recursive layer: parseExpression() 
   │           │                 │    ├─ parseTerm() → 9        
   │           │                 │    ├─ Encounter '+'            
   │           │                 │    ├─ parseTerm() → 1         
   │           │                 │    └─ Return 9+1=10           
   │           │                 │                                
   │           │                 ├─ Consume ')'                  
   │           │                 └─ Return 10                     
   │           │                                                
   │           ├─ Encounter '*'                                  
   │           ├─ Call parseUnary() → return 2                  
   │           └─ Calculate 10*2 = 20                            
   │                                                             
   ├─ Encounter '-'                                              
   ├─ Call parseTerm()                                          
   │     ├─ parseUnary() → 8                                    
   │     ├─ Encounter '/'                                         
   │     ├─ parseUnary() → 4                                    
   │     └─ Calculate 8/4 = 2                                   
   │                                                             
   └─ Calculate 20-2 = 18                                       
                                                             
 Return result: 18                                              
```

---

## 7. Key Design Principles

### 7.1 Recursion: When encountering parentheses, restart analysis

```c
if (is '(') {
    parseExpression();  // Recursion, fresh start
}
```

### 7.2 Loop: When processing consecutive operators of same priority

```c
while (is + or -) {
    // After processing one, continue checking for the next
}
```

### 7.3 Iteration: For unary operator chains

```c
while (is + or -) {  // Not recursion, it's a loop
    count++;
}
```

### 7.4 Why This Design?

| Scenario | Method | Reason |
|----------|--------|--------|
| Parentheses `(...)` | Recursion | Need to restart parsing from lowest priority |
| `+ - * /` chains | Loop | Same priority, left-to-right, sequential |
| `--+` signs | Iteration | Only care about final sign, no recursion needed |

---

## 8. LL(1) Grammar Correspondence

### 8.1 Grammar for This Parser

```
Expression ::= Term (( '+' | '-' ) Term)*
Term       ::= Unary (( '*' | '/' ) Unary)*
Unary      ::= ('+' | '-')* Primary
Primary    ::= NUMBER | '(' Expression ')'
```

### 8.2 Each Rule Corresponds to a Function

```
Expression() ←→ Grammar rule Expression
Term()       ←→ Grammar rule Term
Unary()      ←→ Grammar rule Unary
Primary()    ←→ Grammar rule Primary
```

This is the meaning of **"Recursive Descent"**: **Using mutually recursive function calls to "descend" from the top of the syntax tree to the leaf nodes**.

---

## 9. Actual Code Structure

Function call relationships in this project's parser:

```c
// Main entry point
parserEvaluateExpression()
    │
    ├─ lexerInit()           // Initialize lexer
    ├─ parserNextToken()     // Get first token
    │
    └─ parseExpression()     // ⚡ Start recursive descent
            │
            ├─ parseTerm()
            │       │
            │       └─ parseUnary()
            │               │
            │               └─ parsePrimary()
            │                       │
            │                       ├─ Return number
            │                       └─ Or recursively call parseExpression() for parentheses
            │
            ├─ while (is * or /)
            │       ├─ parseUnary()
            │       └─ applyMul/applyDiv()
            │
            └─ while (is + or -)
                    ├─ parseTerm()
                    └─ applyAdd/applySub()
```

---

## 10. Debugging Tips

### 10.1 Enable Debug Output

```bash
./bin/calculator --debug --debug-level=5
```

### 10.2 Debug Output Example

Input `(9+1)*2` with TRACE level enabled:

```
[PARSER] → parseExpression()
[PARSER] → parseTerm()
[PARSER] → parseUnary()
[PARSER] → parsePrimary()
[PARSER]   → parseExpression()    ← Parentheses recursion, depth=1
[PARSER]   → parseTerm()
[PARSER]   → parseUnary()
[PARSER]   → parsePrimary()
[PARSER] [Step 1] Read number 9
[PARSER]   → parseUnary()
[PARSER]   → parsePrimary()
[PARSER] [Step 2] Read number 1
[PARSER] [Step 3] 9 + 1 = 10
[PARSER] → parseUnary()
[PARSER] → parsePrimary()
[PARSER] [Step 4] Read number 2
[PARSER] [Step 5] 10 * 2 = 20
```

**Indentation meaning:**
- `depth=0`: Outermost expression
- `depth=1`: Expression inside parentheses
- `depth=2`: Deeper nesting (if any)

---

## Conclusion

Recursive descent parsing is an intuitive and efficient parsing technique, especially suitable for **expression parsing** scenarios with clear hierarchical structure and precedence.

Key takeaways:
1. **Layering corresponds to priority**: Expression < Term < Unary < Primary
2. **Recursion for parentheses**: When encountering `(`, recursively call `parseExpression()`
3. **Loops for operator chains**: Operators of the same priority are processed sequentially with loops
4. **Iteration for unary operators**: Use counters instead of recursion to avoid stack overflow

The key to understanding recursive descent is: **Imagine the tree formed by function calls, and complete parsing while "descending" from the root node to the leaf nodes**.
