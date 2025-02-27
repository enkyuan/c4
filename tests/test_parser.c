#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../parser.h"
#include "../lexer.h"
#include "../ast.h"

void test_parser_init() {
    char* source = "int main() { return 0; }";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    
    assert(parser != NULL);
    assert(parser->lexer == lexer);
    assert(parser->had_error == false);
    assert(parser->panic_mode == false);
    
    parser_free(parser);
    lexer_free(lexer);
    printf("test_parser_init: PASSED\n");
}

void test_expression_parsing() {
    char* source = "42 + 23 * 5";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    
    Expression* expr = parse_expression(parser);
    assert(expr != NULL);
    assert(expr->type == NODE_BINARY_OP);
    
    // Check that we got a binary addition
    assert(expr->token->type == TOKEN_PLUS);
    
    // Check left operand (42)
    Expression* left = expr->as.binary.left;
    assert(left->type == NODE_LITERAL);
    assert(left->token->type == TOKEN_INTEGER_LITERAL);
    
    // Check right operand (23 * 5)
    Expression* right = expr->as.binary.right;
    assert(right->type == NODE_BINARY_OP);
    assert(right->token->type == TOKEN_STAR);
    
    free_expression(expr);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_expression_parsing: PASSED\n");
}

void test_statement_parsing() {
    char* source = "if (x > 0) { return x; } else { return -x; }";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    
    Statement* stmt = parse_statement(parser);
    assert(stmt != NULL);
    assert(stmt->type == NODE_IF);
    
    // Check condition
    Expression* condition = stmt->as.if_stmt.condition;
    assert(condition->type == NODE_BINARY_OP);
    assert(condition->token->type == TOKEN_GREATER);
    
    // Check then branch
    Statement* then_branch = stmt->as.if_stmt.then_branch;
    assert(then_branch->type == NODE_COMPOUND);
    
    // Check else branch
    Statement* else_branch = stmt->as.if_stmt.else_branch;
    assert(else_branch != NULL);
    assert(else_branch->type == NODE_COMPOUND);
    
    free_statement(stmt);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_statement_parsing: PASSED\n");
}

void test_error_handling() {
    char* source = "if (x > ) { return x; }";
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    
    Statement* stmt = parse_statement(parser);
    assert(parser->had_error == true);
    assert(parser->error != NULL);
    
    if (stmt != NULL) free_statement(stmt);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_error_handling: PASSED\n");
}

void test_program_parsing() {
    char* source = ""
        "int x = 42;\n"
        "int y = x + 10;\n"
        "return y;\n";
    
    Lexer* lexer = lexer_init(source, "test.c");
    Parser* parser = parser_init(lexer);
    
    Statement* program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == NODE_COMPOUND);
    assert(program->as.compound.count == 3);
    assert(!parser->had_error);
    
    free_statement(program);
    parser_free(parser);
    lexer_free(lexer);
    printf("test_program_parsing: PASSED\n");
}

int main() {
    printf("Running parser tests...\n");
    test_parser_init();
    test_expression_parsing();
    test_statement_parsing();
    test_error_handling();
    test_program_parsing();
    printf("All parser tests passed!\n");
    return 0;
}