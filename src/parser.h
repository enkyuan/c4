// FIXME: return types for parsing functions

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    NODE_INT,
    NODE_BINARY_OP
} NodeType;

typedef struct Node {
    NodeType type;
    int value;
    struct Node *left;
    struct Node *right;
} Node;

extern Token currentToken;

void initParser();

int parseExpression();

#endif

