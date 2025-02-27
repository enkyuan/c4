#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lexer.h"

void test_lexer_init() {
    char* source = "int main() { return 0; }";
    Lexer* lexer = lexer_init(source, "test.c");
    assert(lexer != NULL);
    assert(strcmp(lexer->source, source) == 0);
    assert(lexer->current == 0);
    assert(lexer->line == 1);
    assert(lexer->column == 1);
    lexer_free(lexer);
    printf("test_lexer_init: PASSED\n");
}

void test_basic_tokens() {
    char* source = "int main() {\n    return 42;\n}";
    Lexer* lexer = lexer_init(source, "test.c");
    
    Token* token = lexer_next_token(lexer);
    assert(token->type == TOKEN_INT);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_IDENTIFIER);
    assert(strcmp(token->lexeme, "main") == 0);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_LPAREN);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_RPAREN);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_LBRACE);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_RETURN);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_INTEGER_LITERAL);
    assert(token->value.int_value == 42);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_SEMICOLON);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_RBRACE);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_EOF);
    free(token);
    
    lexer_free(lexer);
    printf("test_basic_tokens: PASSED\n");
}

void test_operators() {
    char* source = "+ - * / = == != < <= > >= && || !";
    Lexer* lexer = lexer_init(source, "test.c");
    
    TokenType expected[] = {
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
        TOKEN_EQUALS, TOKEN_EQUALEQUAL, TOKEN_NOTEQUAL,
        TOKEN_LESS, TOKEN_LESSEQUAL, TOKEN_GREATER, TOKEN_GREATEREQUAL,
        TOKEN_ANDAND, TOKEN_OROR, TOKEN_BANG
    };
    
    for (int i = 0; i < sizeof(expected) / sizeof(TokenType); i++) {
        Token* token = lexer_next_token(lexer);
        assert(token->type == expected[i]);
        free(token);
    }
    
    lexer_free(lexer);
    printf("test_operators: PASSED\n");
}

void test_string_literals() {
    char* source = "\"Hello, World!\" \"Test\\nEscape\"";
    Lexer* lexer = lexer_init(source, "test.c");
    
    Token* token = lexer_next_token(lexer);
    assert(token->type == TOKEN_STRING_LITERAL);
    assert(strcmp(token->value.string_value, "Hello, World!") == 0);
    free(token);
    
    token = lexer_next_token(lexer);
    assert(token->type == TOKEN_STRING_LITERAL);
    assert(strcmp(token->value.string_value, "Test\nEscape") == 0);
    free(token);
    
    lexer_free(lexer);
    printf("test_string_literals: PASSED\n");
}

int main() {
    printf("Running lexer tests...\n");
    test_lexer_init();
    test_basic_tokens();
    test_operators();
    test_string_literals();
    printf("All lexer tests passed!\n");
    return 0;
}