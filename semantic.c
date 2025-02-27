#include "semantic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Forward declarations for semantic analysis functions
static Type* check_binary_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    Type* left = check_expression(analyzer, expr->as.binary.left);
    Type* right = check_expression(analyzer, expr->as.binary.right);
    
    if (!is_type_compatible(left, right)) {
        semantic_error(analyzer, expr->token, "Type mismatch in binary expression");
        return NULL;
    }
    
    return common_type(left, right);
}

static Type* check_unary_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    Type* operand = check_expression(analyzer, expr->as.unary.operand);
    if (operand == NULL) return NULL;
    
    switch (expr->token->type) {
        case TOKEN_MINUS:
        case TOKEN_BANG:
            return operand;
        default:
            semantic_error(analyzer, expr->token, "Invalid unary operator");
            return NULL;
    }
}

static Type* check_literal_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    switch (expr->token->type) {
        case TOKEN_INTEGER_LITERAL:
            return create_basic_type(TYPE_INT, false, false);
        case TOKEN_FLOAT_LITERAL:
            return create_basic_type(TYPE_FLOAT, false, false);
        case TOKEN_STRING_LITERAL:
            return create_basic_type(TYPE_CHAR, true, false);
        default:
            semantic_error(analyzer, expr->token, "Invalid literal type");
            return NULL;
    }
}

static Type* check_identifier_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    SymbolEntry* entry = lookup_symbol(analyzer, expr->token->lexeme);
    if (entry == NULL) {
        semantic_error(analyzer, expr->token, "Undefined variable");
        return NULL;
    }
    return entry->type;
}

static Type* check_call_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    Type* callee_type = check_expression(analyzer, expr->as.call.callee);
    if (callee_type == NULL) return NULL;
    
    if (callee_type->kind != TYPE_FUNCTION) {
        semantic_error(analyzer, expr->token, "Cannot call non-function type");
        return NULL;
    }
    
    return callee_type->info.func.return_type;
}

static void check_if_statement(SemanticAnalyzer* analyzer, Statement* stmt) {
    Type* condition = check_expression(analyzer, stmt->as.if_stmt.condition);
    if (condition != NULL && condition->kind != TYPE_BOOL) {
        semantic_error(analyzer, stmt->token, "Condition must be a boolean expression");
    }
    
    check_statement(analyzer, stmt->as.if_stmt.then_branch);
    if (stmt->as.if_stmt.else_branch != NULL) {
        check_statement(analyzer, stmt->as.if_stmt.else_branch);
    }
}

static void check_loop_statement(SemanticAnalyzer* analyzer, Statement* stmt) {
    bool was_in_loop = analyzer->in_loop;
    analyzer->in_loop = true;
    
    if (stmt->type == NODE_WHILE) {
        Type* condition = check_expression(analyzer, stmt->as.while_stmt.condition);
        if (condition != NULL && condition->kind != TYPE_BOOL) {
            semantic_error(analyzer, stmt->token, "Condition must be a boolean expression");
        }
        check_statement(analyzer, stmt->as.while_stmt.body);
    } else if (stmt->type == NODE_FOR) {
        if (stmt->as.for_stmt.initializer != NULL) {
            check_statement(analyzer, stmt->as.for_stmt.initializer);
        }
        if (stmt->as.for_stmt.condition != NULL) {
            Type* condition = check_expression(analyzer, stmt->as.for_stmt.condition);
            if (condition != NULL && condition->kind != TYPE_BOOL) {
                semantic_error(analyzer, stmt->token, "Condition must be a boolean expression");
            }
        }
        if (stmt->as.for_stmt.increment != NULL) {
            check_statement(analyzer, stmt->as.for_stmt.increment);
        }
        check_statement(analyzer, stmt->as.for_stmt.body);
    }
    
    analyzer->in_loop = was_in_loop;
}

static void check_return_statement(SemanticAnalyzer* analyzer, Statement* stmt) {
    if (analyzer->current_function_return_type == NULL) {
        semantic_error(analyzer, stmt->token, "Return statement outside of function");
        return;
    }
    
    if (stmt->as.return_stmt.value != NULL) {
        Type* value_type = check_expression(analyzer, stmt->as.return_stmt.value);
        if (value_type != NULL && !is_type_compatible(analyzer->current_function_return_type, value_type)) {
            semantic_error(analyzer, stmt->token, "Return value type does not match function return type");
        }
    } else if (analyzer->current_function_return_type->kind != TYPE_VOID) {
        semantic_error(analyzer, stmt->token, "Function must return a value");
    }
}

// Initialize semantic analyzer
SemanticAnalyzer* semantic_init(void) {
    SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
    analyzer->current_scope = NULL;
    analyzer->current_function_return_type = NULL;
    analyzer->in_loop = false;
    analyzer->had_error = false;
    analyzer->filename = NULL;
    return analyzer;
}

void semantic_free(SemanticAnalyzer* analyzer) {
    while (analyzer->current_scope != NULL) {
        leave_scope(analyzer);
    }
    free(analyzer->filename);
    free(analyzer);
}

// Scope management
void enter_scope(SemanticAnalyzer* analyzer) {
    Scope* scope = malloc(sizeof(Scope));
    scope->entries = NULL;
    scope->parent = analyzer->current_scope;
    scope->level = analyzer->current_scope ? analyzer->current_scope->level + 1 : 0;
    analyzer->current_scope = scope;
}

void leave_scope(SemanticAnalyzer* analyzer) {
    if (analyzer->current_scope == NULL) return;
    
    // Free all symbols in current scope
    SymbolEntry* entry = analyzer->current_scope->entries;
    while (entry != NULL) {
        SymbolEntry* next = entry->next;
        free(entry->name);
        if (entry->kind == SYMBOL_FUNCTION) {
            free(entry->info.func.param_types);
        }
        free(entry);
        entry = next;
    }
    
    Scope* parent = analyzer->current_scope->parent;
    free(analyzer->current_scope);
    analyzer->current_scope = parent;
}

// Symbol table operations
SymbolEntry* declare_symbol(SemanticAnalyzer* analyzer, char* name, Type* type, int kind) {
    // Check if symbol already exists in current scope
    if (lookup_symbol_current_scope(analyzer, name)) {
        return NULL; // Symbol already declared in current scope
    }
    
    SymbolEntry* entry = malloc(sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->type = type;
    entry->kind = kind;
    entry->is_defined = false;
    entry->next = analyzer->current_scope->entries;
    analyzer->current_scope->entries = entry;
    
    return entry;
}

SymbolEntry* lookup_symbol(SemanticAnalyzer* analyzer, char* name) {
    for (Scope* scope = analyzer->current_scope; scope != NULL; scope = scope->parent) {
        for (SymbolEntry* entry = scope->entries; entry != NULL; entry = entry->next) {
            if (strcmp(entry->name, name) == 0) {
                return entry;
            }
        }
    }
    return NULL;
}

SymbolEntry* lookup_symbol_current_scope(SemanticAnalyzer* analyzer, char* name) {
    for (SymbolEntry* entry = analyzer->current_scope->entries;
         entry != NULL;
         entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
    }
    return NULL;
}

// Type checking functions
Type* check_expression(SemanticAnalyzer* analyzer, Expression* expr) {
    switch (expr->type) {
        case NODE_BINARY_OP:
            return check_binary_expression(analyzer, expr);
        case NODE_UNARY_OP:
            return check_unary_expression(analyzer, expr);
        case NODE_LITERAL:
            return check_literal_expression(analyzer, expr);
        case NODE_IDENTIFIER:
            return check_identifier_expression(analyzer, expr);
        case NODE_CALL:
            return check_call_expression(analyzer, expr);
        default:
            return NULL;
    }
}

void check_statement(SemanticAnalyzer* analyzer, Statement* stmt) {
    switch (stmt->type) {
        case NODE_IF:
            check_if_statement(analyzer, stmt);
            break;
        case NODE_WHILE:
        case NODE_DO_WHILE:
        case NODE_FOR:
            check_loop_statement(analyzer, stmt);
            break;
        case NODE_RETURN:
            check_return_statement(analyzer, stmt);
            break;
        case NODE_DECLARATION:
            check_declaration(analyzer, stmt);
            break;
        case NODE_COMPOUND:
            enter_scope(analyzer);
            for (int i = 0; i < stmt->as.compound.count; i++) {
                check_statement(analyzer, stmt->as.compound.statements[i]);
            }
            leave_scope(analyzer);
            break;
        default:
            break;
    }
}

void check_declaration(SemanticAnalyzer* analyzer, Statement* stmt) {
    if (stmt->type != NODE_DECLARATION) return;
    
    // Check if variable name is already declared in current scope
    SymbolEntry* existing = lookup_symbol_current_scope(analyzer, stmt->as.declaration.name->lexeme);
    if (existing != NULL) {
        semantic_error(analyzer, stmt->token, "Variable already declared in this scope");
        return;
    }
    
    // Check initializer expression if present
    Type* init_type = NULL;
    if (stmt->as.declaration.initializer != NULL) {
        init_type = check_expression(analyzer, stmt->as.declaration.initializer);
        if (init_type == NULL) return;
    }
    
    // Declare the variable in current scope
    Type* var_type = create_basic_type(TYPE_INT, false, false); // Default to int for now
    declare_symbol(analyzer, stmt->as.declaration.name->lexeme, var_type, SYMBOL_VARIABLE);
}

// Type compatibility and conversion
bool is_type_compatible(Type* left, Type* right) {
    if (left == NULL || right == NULL) return false;
    
    // Same type
    if (left->kind == right->kind) return true;
    
    // Pointer compatibility
    if (left->kind == TYPE_POINTER && right->kind == TYPE_POINTER) {
        return is_type_compatible(left->info.base, right->info.base);
    }
    
    // Numeric type compatibility
    if ((left->kind == TYPE_INT || left->kind == TYPE_FLOAT || left->kind == TYPE_DOUBLE) &&
        (right->kind == TYPE_INT || right->kind == TYPE_FLOAT || right->kind == TYPE_DOUBLE)) {
        return true;
    }
    
    return false;
}

Type* common_type(Type* left, Type* right) {
    if (!is_type_compatible(left, right)) return NULL;
    
    // Same type
    if (left->kind == right->kind) return left;
    
    // Numeric type promotion
    if (left->kind == TYPE_DOUBLE || right->kind == TYPE_DOUBLE) {
        return create_basic_type(TYPE_DOUBLE, false, false);
    }
    if (left->kind == TYPE_FLOAT || right->kind == TYPE_FLOAT) {
        return create_basic_type(TYPE_FLOAT, false, false);
    }
    
    return create_basic_type(TYPE_INT, false, false);
}

Expression* implicit_cast(Expression* expr, Type* target_type) {
    if (expr == NULL || target_type == NULL) return NULL;
    if (is_type_compatible(expr->expr_type, target_type)) {
        Expression* cast = malloc(sizeof(Expression));
        cast->type = NODE_CAST;
        cast->expr_type = target_type;
        cast->token = expr->token;
        cast->as.unary.operand = expr;
        return cast;
    }
    return NULL;
}

// Error reporting
void semantic_error(SemanticAnalyzer* analyzer, Token* token, const char* message) {
    analyzer->had_error = true;
    fprintf(stderr, "%s:%d:%d: error: %s\n",
            analyzer->filename,
            token->line,
            token->column,
            message);
}