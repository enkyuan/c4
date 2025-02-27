#include "ast.h"
#include <stdlib.h>
#include <string.h>

// Expression node creation functions
Expression* create_binary_expr(Expression* left, Expression* right, TokenType op, Token* token) {
    Expression* expr = malloc(sizeof(Expression));
    expr->type = NODE_BINARY_OP;
    expr->token = token;
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

Expression* create_unary_expr(Expression* operand, TokenType op, bool prefix, Token* token) {
    Expression* expr = malloc(sizeof(Expression));
    expr->type = NODE_UNARY_OP;
    expr->token = token;
    expr->as.unary.operand = operand;
    expr->as.unary.prefix = prefix;
    return expr;
}

Expression* create_literal_expr(Token* token) {
    Expression* expr = malloc(sizeof(Expression));
    expr->type = NODE_LITERAL;
    expr->token = token;
    return expr;
}

Expression* create_identifier_expr(Token* token) {
    Expression* expr = malloc(sizeof(Expression));
    expr->type = NODE_IDENTIFIER;
    expr->token = token;
    return expr;
}

Expression* create_call_expr(Expression* callee, Expression** args, int arg_count, Token* token) {
    Expression* expr = malloc(sizeof(Expression));
    expr->type = NODE_CALL;
    expr->token = token;
    expr->as.call.callee = callee;
    expr->as.call.args = args;
    expr->as.call.arg_count = arg_count;
    return expr;
}

// Statement node creation functions
Statement* create_if_stmt(Expression* condition, Statement* then_branch, Statement* else_branch, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_IF;
    stmt->token = token;
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;
    return stmt;
}

Statement* create_while_stmt(Expression* condition, Statement* body, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_WHILE;
    stmt->token = token;
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;
    return stmt;
}

Statement* create_for_stmt(Statement* initializer, Expression* condition, Statement* increment, Statement* body, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_FOR;
    stmt->token = token;
    stmt->as.for_stmt.initializer = initializer;
    stmt->as.for_stmt.condition = condition;
    stmt->as.for_stmt.increment = increment;
    stmt->as.for_stmt.body = body;
    return stmt;
}

Statement* create_return_stmt(Expression* value, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_RETURN;
    stmt->token = token;
    stmt->as.return_stmt.value = value;
    return stmt;
}

Statement* create_compound_stmt(Statement** statements, int count, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_COMPOUND;
    stmt->token = token;
    stmt->as.compound.statements = statements;
    stmt->as.compound.count = count;
    return stmt;
}

// Type creation functions
Type* create_basic_type(TypeKind kind, bool is_const, bool is_volatile) {
    Type* type = malloc(sizeof(Type));
    type->kind = kind;
    type->is_const = is_const;
    type->is_volatile = is_volatile;
    return type;
}

Type* create_pointer_type(Type* base, bool is_const, bool is_volatile) {
    Type* type = malloc(sizeof(Type));
    type->kind = TYPE_POINTER;
    type->is_const = is_const;
    type->is_volatile = is_volatile;
    type->info.base = base;
    return type;
}

Type* create_array_type(Type* elem_type, int size, bool is_const, bool is_volatile) {
    Type* type = malloc(sizeof(Type));
    type->kind = TYPE_ARRAY;
    type->is_const = is_const;
    type->is_volatile = is_volatile;
    type->info.array.elem_type = elem_type;
    type->info.array.size = size;
    return type;
}

Type* create_function_type(Type* return_type, Type** param_types, int param_count) {
    Type* type = malloc(sizeof(Type));
    type->kind = TYPE_FUNCTION;
    type->is_const = false;
    type->is_volatile = false;
    type->info.func.return_type = return_type;
    type->info.func.param_types = param_types;
    type->info.func.param_count = param_count;
    return type;
}

// Memory management functions
void free_expression(Expression* expr) {
    if (expr == NULL) return;
    
    switch (expr->type) {
        case NODE_BINARY_OP:
            free_expression(expr->as.binary.left);
            free_expression(expr->as.binary.right);
            break;
        case NODE_UNARY_OP:
            free_expression(expr->as.unary.operand);
            break;
        case NODE_CALL:
            free_expression(expr->as.call.callee);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                free_expression(expr->as.call.args[i]);
            }
            free(expr->as.call.args);
            break;
        default:
            break;
    }
    
    free(expr);
}

void free_statement(Statement* stmt) {
    if (stmt == NULL) return;
    
    switch (stmt->type) {
        case NODE_IF:
            free_expression(stmt->as.if_stmt.condition);
            free_statement(stmt->as.if_stmt.then_branch);
            free_statement(stmt->as.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            free_expression(stmt->as.while_stmt.condition);
            free_statement(stmt->as.while_stmt.body);
            break;
        case NODE_FOR:
            free_statement(stmt->as.for_stmt.initializer);
            free_expression(stmt->as.for_stmt.condition);
            free_statement(stmt->as.for_stmt.increment);
            free_statement(stmt->as.for_stmt.body);
            break;
        case NODE_RETURN:
            free_expression(stmt->as.return_stmt.value);
            break;
        case NODE_COMPOUND:
            for (int i = 0; i < stmt->as.compound.count; i++) {
                free_statement(stmt->as.compound.statements[i]);
            }
            free(stmt->as.compound.statements);
            break;
        default:
            break;
    }
    
    free(stmt);
}

void free_type(Type* type) {
    if (type == NULL) return;
    
    switch (type->kind) {
        case TYPE_POINTER:
            free_type(type->info.base);
            break;
        case TYPE_ARRAY:
            free_type(type->info.array.elem_type);
            break;
        case TYPE_FUNCTION:
            free_type(type->info.func.return_type);
            for (int i = 0; i < type->info.func.param_count; i++) {
                free_type(type->info.func.param_types[i]);
            }
            free(type->info.func.param_types);
            break;
        default:
            break;
    }
    
    free(type);
}

Statement* create_expression_stmt(Expression* expr, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_EXPRESSION;
    stmt->token = token;
    stmt->as.expression.expr = expr;
    return stmt;
}

Statement* create_var_stmt(Token* name, Expression* initializer, Token* token) {
    Statement* stmt = malloc(sizeof(Statement));
    stmt->type = NODE_DECLARATION;
    stmt->token = token;
    stmt->as.declaration.name = name;
    stmt->as.declaration.initializer = initializer;
    return stmt;
}