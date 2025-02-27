#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../semantic.h"
#include "../parser.h"
#include "../lexer.h"
#include "../ast.h"

void test_semantic_init() {
    SemanticAnalyzer* analyzer = semantic_init();
    assert(analyzer != NULL);
    assert(analyzer->had_error == false);
    semantic_free(analyzer);
    printf("test_semantic_init: PASSED\n");
}

void test_type_checking() {
    char* source = "int x = 42.5;";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    SemanticAnalyzer* analyzer = semantic_init();
    analyzer->filename = strdup("test.c");
    
    Statement* stmt = parse_declaration(parser);
    check_statement(analyzer, stmt);
    
    assert(analyzer->had_error == true); // Should error on float to int conversion
    
    free_statement(stmt);
    semantic_free(analyzer);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_type_checking: PASSED\n");
}

void test_scope_analysis() {
    char* source = ""
        "{"
        "    int x = 10;"
        "    {"
        "        int x = 20;"
        "    }"
        "    x = 30;"
        "}";
    
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    SemanticAnalyzer* analyzer = semantic_init();
    analyzer->filename = strdup("test.c");
    
    Statement* stmt = parse_statement(parser);
    check_statement(analyzer, stmt);
    
    assert(analyzer->had_error == false); // Valid nested scopes
    
    free_statement(stmt);
    semantic_free(analyzer);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_scope_analysis: PASSED\n");
}

void test_undefined_variable() {
    char* source = "x = 42;";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    SemanticAnalyzer* analyzer = semantic_init();
    analyzer->filename = strdup("test.c");
    
    Statement* stmt = parse_statement(parser);
    check_statement(analyzer, stmt);
    
    assert(analyzer->had_error == true); // Should error on undefined variable
    
    free_statement(stmt);
    semantic_free(analyzer);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_undefined_variable: PASSED\n");
}

void test_function_call() {
    char* source = ""
        "int add(int x, int y) { return x + y; }"
        "int main() {"
        "    return add(40, 2);"
        "}";
    
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    SemanticAnalyzer* analyzer = semantic_init();
    analyzer->filename = strdup("test.c");
    
    Statement* program = parse_program(parser);
    check_statement(analyzer, program);
    
    assert(analyzer->had_error == false); // Valid function call
    
    free_statement(program);
    semantic_free(analyzer);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_function_call: PASSED\n");
}

int main() {
    printf("Running semantic analyzer tests...\n");
    test_semantic_init();
    test_type_checking();
    test_scope_analysis();
    test_undefined_variable();
    test_function_call();
    printf("All semantic analyzer tests passed!\n");
    return 0;
}