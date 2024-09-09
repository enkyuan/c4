// TODO: document this code - explain each line

#include "lexer.h"

char *src;
char *current;

void initLexer(char *source) {
    src = source;
    current = src;
}

// Implement your lexer here
Token getNextToken() {
    if (*current == '\0') {
        return (Token) {TOKEN_EOF, 0};
    }

    if (*current == '+') {
        current++;
        return (Token) {TOKEN_PLUS, 0};
    }

    if (*current >= '0' && *current <= '9') {
        int value = 0;
        while (*current >= '0' && *current <= '9') {
            value = value * 10 + (*current - '0');
            current++;
        }
        return (Token) {TOKEN_INT, value};
    }

    return (Token) {TOKEN_EOF, 0};
}

