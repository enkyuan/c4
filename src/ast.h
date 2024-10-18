#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    AST_LITERAL,
    AST_BINARY_OP,
    AST_IDENTIFIER,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int literalValue;
        struct {
            char *name;
        } identifier;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            char *operator;
        } binaryOp;
    } data;
} ASTNode;

ASTNode* createLiteralNode(int value);
ASTNode* createBinaryOpNode(ASTNode *left, ASTNode *right, char *operator);
ASTNode* createIdentifierNode(char *name);
void freeAST(ASTNode *node);

#endif 
