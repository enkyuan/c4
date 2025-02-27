# c4

## About

c4 is a self-contained C compiler written in C, designed to compile a subset of C11 into x86_64 assembly. The project aims to provide a clear, educational implementation of a modern compiler while maintaining good performance and generating efficient code.

## Features

- Lexical analysis with support for C11 tokens and keywords
- Recursive descent parser for C grammar
- Comprehensive error reporting with line and column information
- Semantic analysis including type checking and symbol resolution
- x86_64 code generation
- Support for basic C constructs:
  - Variables and basic types (int, float, char)
  - Control flow (if, while, for)
  - Functions and function calls
  - Basic operators and expressions

## Building

Requirements:
- GCC or Clang
- Make

To build the project:

```bash
make
```

## Usage

Compile a C source file:

```bash
./c4 input.c -o output
```

## Project Structure

- `lexer.{h,c}`: Lexical analysis
- `parser.{h,c}`: Recursive descent parser
- `ast.{h,c}`: Abstract syntax tree definitions
- `semantic.{h,c}`: Semantic analysis and type checking
- `codegen.{h,c}`: x86_64 code generation
- `tests/`: Test suite

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
