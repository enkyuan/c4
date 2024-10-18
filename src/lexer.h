#include <stdio.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_INT,
    TOKEN_PLUS,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int value;
} Token;

extern char *src;
extern char *current;

void initLexer(char *source);
Token getNextToken();

#endif


