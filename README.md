# Compiler
*Course Project of Compiler Principle (2021-Spring-Summer)*

A toy compiler for our self-designed simple programming language CMM (C--).

## Structure

* `lex` and `yacc` parser
* AST Generator
* LLVM IR Generator
* Test Cases
    - Quicksort
    - Matrix Multiplication
    - Auto Advisor for Course Enrollment

## Features
* Global / Local variable declaration
* Function definition and call
* Arithmetical / Logical expression evaluation
* Control flow statments (`if`, `loop`, `break`, `continue`, `return`...)
* Arrays (with arbitrary dimensions)
    - Support complex indexing like `a[b[c[x]]][d[y]]`
* C-Style IO
* Closure
    - Value capturing
    - Support escaping closure
* Passing function as an argument to function call
    - Make implement `map` and `filter` possible

## Build
CMake 3 and LLVM 12.0.0 are necessary.
You may need to specify your LLVM installation path to CMake.

``` bash
cmake .
make
```

## Run
```
./compiler ./source.cmm 
```
Then a LLVM IR assembly file will be produced.

Option: `-e` to produce a executable. You can also use `lli` to interpret the IR file or `llc` to compiler it to assembly of any target.