#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "ast.h"
#include "error.h"

ASTNode* parseExpression();
ASTNode* parseTerm();
ASTNode* parseFactor();

ASTNode* parseExpression() {
    ASTNode* left = parseTerm();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        TokenType operator = currentToken.type;
        getNextToken(); // Consume the operator
        ASTNode* right = parseTerm();
        left = createBinaryOpNode(left, right, operator);
    }
    return left;
}

ASTNode* parseTerm() {
    ASTNode* left = parseFactor();
    while (currentToken.type == TOKEN_MULTIPLY || currentToken.type == TOKEN_DIVIDE) {
        TokenType operator = currentToken.type;
        getNextToken(); // Consume the operator
        ASTNode* right = parseFactor();
        left = createBinaryOpNode(left, right, operator);
    }
    return left;
}

ASTNode* parseFactor() {
    if (currentToken.type == TOKEN_INT) {
        int value = currentToken.value;
        getNextToken(); // Consume the integer
        return createLiteralNode(value);
    } else if (currentToken.type == TOKEN_LPAREN) {
        getNextToken(); // Consume the '('
        ASTNode* node = parseExpression();
        if (currentToken.type != TOKEN_RPAREN) {
            handleFatalError(ERR_SYNTAX, "Expected ')'");
        }
        getNextToken(); // Consume the ')'
        return node;
    } else {
        handleFatalError(ERR_SYNTAX, "Unexpected token");
        return NULL; // Unreachable, but needed to satisfy the compiler
    }
}

ASTNode* initParse() {
    getNextToken(); // Initialize the first token
    return parseExpression();
}
