# Compiler
*Course Project of Compiler Principle (2021-Spring-Summer)*

A toy compiler for our self-designed simple programming language CMM (C--).

## Structure

* `lex` and `yacc` parser by [Fusion](https://github.com/Fusion4233919)
* AST Generator & Semantic Analyzer by [Fusion](https://github.com/Fusion4233919)
* LLVM IR Generator by [andongfan](https://github.com/andongfan/)
* Test Cases by [Flaze](https://github.com/flazenaive)
    - Quicksort
    - Matrix Multiplication
    - Auto Advisor for Course Enrollment

## Features
* Simple semantic analysis
    - Attribute calculation
    - Symbol table
    - Type checking
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