#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdbool.h>

// Node types
typedef enum {
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_CALL,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_DECLARATION,
    NODE_COMPOUND,
    NODE_CAST,
    NODE_EXPRESSION
} NodeType;

// Type kinds
typedef enum {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_FUNCTION
} TypeKind;

// Forward declarations
typedef struct Type Type;
typedef struct Expression Expression;
typedef struct Statement Statement;

// Function type info
typedef struct {
    Type* return_type;
    Type** param_types;
    int param_count;
} FunctionType;

// Type structure
struct Type {
    TypeKind kind;
    bool is_const;
    bool is_volatile;
    union {
        Type* base;          // For pointer types
        FunctionType func;   // For function types
        struct {
            Type* elem_type; // For array types
            int size;
        } array;
    } info;
};

// Expression structure
struct Expression {
    NodeType type;
    Type* expr_type;
    Token* token;
    union {
        struct {
            Expression* left;
            Expression* right;
        } binary;
        struct {
            Expression* operand;
            bool prefix;
        } unary;
        struct {
            Expression* callee;
            Expression** args;
            int arg_count;
        } call;
    } as;
};

// Statement structure
struct Statement {
    NodeType type;
    Token* token;
    union {
        struct {
            Expression* condition;
            Statement* then_branch;
            Statement* else_branch;
        } if_stmt;
        struct {
            Expression* condition;
            Statement* body;
        } while_stmt;
        struct {
            Statement* initializer;
            Expression* condition;
            Statement* increment;
            Statement* body;
        } for_stmt;
        struct {
            Expression* value;
        } return_stmt;
        struct {
            Statement** statements;
            int count;
        } compound;
        struct {
            Expression* expr;
        } expression;
        struct {
            Token* name;
            Expression* initializer;
        } declaration;
    } as;
};

// AST creation functions
Expression* create_binary_expr(Expression* left, Expression* right, TokenType op, Token* token);
Expression* create_unary_expr(Expression* operand, TokenType op, bool prefix, Token* token);
Expression* create_literal_expr(Token* token);
Expression* create_identifier_expr(Token* token);
Expression* create_call_expr(Expression* callee, Expression** args, int arg_count, Token* token);

Statement* create_if_stmt(Expression* condition, Statement* then_branch, Statement* else_branch, Token* token);
Statement* create_while_stmt(Expression* condition, Statement* body, Token* token);
Statement* create_for_stmt(Statement* initializer, Expression* condition, Statement* increment, Statement* body, Token* token);
Statement* create_return_stmt(Expression* value, Token* token);
Statement* create_compound_stmt(Statement** statements, int count, Token* token);
Statement* create_expression_stmt(Expression* expr, Token* token);
Statement* create_var_stmt(Token* name, Expression* initializer, Token* token);

// Type creation functions
Type* create_basic_type(TypeKind kind, bool is_const, bool is_volatile);
Type* create_pointer_type(Type* base, bool is_const, bool is_volatile);
Type* create_array_type(Type* elem_type, int size, bool is_const, bool is_volatile);
Type* create_function_type(Type* return_type, Type** param_types, int param_count);

// Memory management
void free_expression(Expression* expr);
void free_statement(Statement* stmt);
void free_type(Type* type);

#endif // AST_H