#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <stdbool.h>

// Symbol table entry structure
typedef struct SymbolEntry {
    char* name;
    Type* type;
    enum {
        SYMBOL_VARIABLE,
        SYMBOL_FUNCTION,
        SYMBOL_TYPE,
        SYMBOL_CONSTANT
    } kind;
    bool is_defined;
    union {
        // For variables
        struct {
            bool is_global;
            int offset;
        } var;
        // For functions
        struct {
            Type** param_types;
            int param_count;
            bool is_variadic;
        } func;
    } info;
    struct SymbolEntry* next;
} SymbolEntry;

// Symbol table scope structure
typedef struct Scope {
    SymbolEntry* entries;
    struct Scope* parent;
    int level;
} Scope;

// Semantic analyzer state
typedef struct {
    Scope* current_scope;
    Type* current_function_return_type;
    bool in_loop;
    bool had_error;
    char* filename;
} SemanticAnalyzer;

// Semantic analyzer interface functions
SemanticAnalyzer* semantic_init(void);
void semantic_free(SemanticAnalyzer* analyzer);

// Scope management
void enter_scope(SemanticAnalyzer* analyzer);
void leave_scope(SemanticAnalyzer* analyzer);

// Symbol table operations
SymbolEntry* declare_symbol(SemanticAnalyzer* analyzer, char* name, Type* type, int kind);
SymbolEntry* lookup_symbol(SemanticAnalyzer* analyzer, char* name);
SymbolEntry* lookup_symbol_current_scope(SemanticAnalyzer* analyzer, char* name);

// Type checking functions
Type* check_expression(SemanticAnalyzer* analyzer, Expression* expr);
void check_statement(SemanticAnalyzer* analyzer, Statement* stmt);
void check_declaration(SemanticAnalyzer* analyzer, Statement* decl);

// Type compatibility and conversion
bool is_type_compatible(Type* left, Type* right);
Type* common_type(Type* left, Type* right);
Expression* implicit_cast(Expression* expr, Type* target_type);

// Error reporting
void semantic_error(SemanticAnalyzer* analyzer, Token* token, const char* message);

#endif // SEMANTIC_H