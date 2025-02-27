#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Keyword lookup table
static struct {
    const char* keyword;
    TokenType type;
} keywords[] = {
    {"auto", TOKEN_AUTO},
    {"break", TOKEN_BREAK},
    {"case", TOKEN_CASE},
    {"char", TOKEN_CHAR},
    {"const", TOKEN_CONST},
    {"continue", TOKEN_CONTINUE},
    {"default", TOKEN_DEFAULT},
    {"do", TOKEN_DO},
    {"double", TOKEN_DOUBLE},
    {"else", TOKEN_ELSE},
    {"enum", TOKEN_ENUM},
    {"extern", TOKEN_EXTERN},
    {"float", TOKEN_FLOAT},
    {"for", TOKEN_FOR},
    {"goto", TOKEN_GOTO},
    {"if", TOKEN_IF},
    {"inline", TOKEN_INLINE},
    {"int", TOKEN_INT},
    {"long", TOKEN_LONG},
    {"register", TOKEN_REGISTER},
    {"restrict", TOKEN_RESTRICT},
    {"return", TOKEN_RETURN},
    {"short", TOKEN_SHORT},
    {"signed", TOKEN_SIGNED},
    {"sizeof", TOKEN_SIZEOF},
    {"static", TOKEN_STATIC},
    {"struct", TOKEN_STRUCT},
    {"switch", TOKEN_SWITCH},
    {"typedef", TOKEN_TYPEDEF},
    {"union", TOKEN_UNION},
    {"unsigned", TOKEN_UNSIGNED},
    {"void", TOKEN_VOID},
    {"volatile", TOKEN_VOLATILE},
    {"while", TOKEN_WHILE},
    {NULL, 0}
};

// Helper functions
static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

static char peek(Lexer* lexer) {
    if (lexer->current >= lexer->source_length) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->source_length) return '\0';
    return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->source[lexer->current - 1];
}

static bool match(Lexer* lexer, char expected) {
    if (peek(lexer) != expected) return false;
    advance(lexer);
    return true;
}

// Token creation helper
static Token* make_token(Lexer* lexer, TokenType type, size_t start, size_t length) {
    Token* token = malloc(sizeof(Token));
    token->type = type;
    token->line = lexer->line;
    token->column = start + 1;
    
    token->lexeme = malloc(length + 1);
    strncpy(token->lexeme, &lexer->source[start], length);
    token->lexeme[length] = '\0';
    
    return token;
}

// Lexer interface implementation
Lexer* lexer_init(char* source, char* filename) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = strlen(source);
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = strdup(filename);
    return lexer;
}

void lexer_free(Lexer* lexer) {
    free(lexer->filename);
    free(lexer);
}

// Skip whitespace and comments
static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // Line comment
                    while (peek(lexer) != '\n' && peek(lexer) != '\0') advance(lexer);
                } else if (peek_next(lexer) == '*') {
                    // Block comment
                    advance(lexer); // Skip /
                    advance(lexer); // Skip *
                    while (!(peek(lexer) == '*' && peek_next(lexer) == '/')) {
                        if (peek(lexer) == '\n') {
                            lexer->line++;
                            lexer->column = 1;
                        }
                        if (peek(lexer) == '\0') return; // Unterminated comment
                        advance(lexer);
                    }
                    advance(lexer); // Skip *
                    advance(lexer); // Skip /
                } else return;
                break;
            default:
                return;
        }
    }
}

// Scan an identifier or keyword
static Token* identifier(Lexer* lexer) {
    size_t start = lexer->current;  // Start from the current position
    advance(lexer);  // Consume the first character
    while (is_alnum(peek(lexer))) advance(lexer);
    
    size_t length = lexer->current - start;
    
    // Check if it's a keyword
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strlen(keywords[i].keyword) == length &&
            strncmp(&lexer->source[start], keywords[i].keyword, length) == 0) {
            return make_token(lexer, keywords[i].type, start, length);
        }
    }
    
    return make_token(lexer, TOKEN_IDENTIFIER, start, length);
}


// Scan a number (integer or float)
static Token* number(Lexer* lexer) {
    size_t start = lexer->current - 1;
    TokenType type = TOKEN_INTEGER_LITERAL;
    
    while (is_digit(peek(lexer))) advance(lexer);
    
    // Look for a decimal point
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        type = TOKEN_FLOAT_LITERAL;
        advance(lexer); // Consume the .
        while (is_digit(peek(lexer))) advance(lexer);
    }
    
    size_t length = lexer->current - start;
    Token* token = make_token(lexer, type, start, length);
    
    if (type == TOKEN_INTEGER_LITERAL) {
        token->value.int_value = strtoll(token->lexeme, NULL, 10);
    } else {
        token->value.float_value = strtod(token->lexeme, NULL);
    }
    return token;
}

// Scan a string literal
static Token* string(Lexer* lexer) {
    size_t start = lexer->current - 1;  // Include the opening quote
    
    while (peek(lexer) != '"' && peek(lexer) != '\0') {
        if (peek(lexer) == '\n') lexer->line++;
        if (peek(lexer) == '\\') advance(lexer); // Skip escape character
        advance(lexer);
    }
    
    if (peek(lexer) == '\0') {
        // Unterminated string
        return make_token(lexer, TOKEN_ERROR, start, 1);
    }
    
    advance(lexer); // Closing quote
    size_t length = lexer->current - start;
    
    Token* token = make_token(lexer, TOKEN_STRING_LITERAL, start, length);
    
    // Remove the quotes
    token->value.string_value = malloc(length - 1);
    strncpy(token->value.string_value, &lexer->source[start + 1], length - 2);
    token->value.string_value[length - 2] = '\0';
    
    return token;
}

// Main lexer function
Token* lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    if (lexer->current >= lexer->source_length) {
        return make_token(lexer, TOKEN_EOF, lexer->current, 0);
    }
    
    size_t token_start = lexer->current;
    lexer->column = token_start + 1;  // Set column to the start of this token
    char c = advance(lexer);
    
    if (is_alpha(c)) {
        lexer->current--; // Move back to start of identifier
        return identifier(lexer);
    }
    if (is_digit(c)) {
        lexer->current--; // Move back to start of number
        return number(lexer);
    }
    
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN, token_start, 1);
        case ')': return make_token(lexer, TOKEN_RPAREN, token_start, 1);
        case '{': return make_token(lexer, TOKEN_LBRACE, token_start, 1);
        case '}': return make_token(lexer, TOKEN_RBRACE, token_start, 1);
        case '[': return make_token(lexer, TOKEN_LBRACKET, token_start, 1);
        case ']': return make_token(lexer, TOKEN_RBRACKET, token_start, 1);
        case ';': return make_token(lexer, TOKEN_SEMICOLON, token_start, 1);
        case ',': return make_token(lexer, TOKEN_COMMA, token_start, 1);
        case '.': return make_token(lexer, TOKEN_DOT, token_start, 1);
        case '-': {
            size_t len = 1 + (peek(lexer) == '>' || peek(lexer) == '-' ? 1 : 0);
            TokenType type = match(lexer, '>') ? TOKEN_MINUSEQUAL :
                            match(lexer, '-') ? TOKEN_MINUSMINUS : TOKEN_MINUS;
            return make_token(lexer, type, token_start, len);
        }
        case '+': {
            size_t len = 1 + (peek(lexer) == '+' || peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '+') ? TOKEN_PLUSPLUS :
                            match(lexer, '=') ? TOKEN_PLUSEQUAL : TOKEN_PLUS;
            return make_token(lexer, type, token_start, len);
        }
        case '/': {
            size_t len = 1 + (peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_SLASHEQUAL : TOKEN_SLASH;
            return make_token(lexer, type, token_start, len);
        }
        case '*': {
            size_t len = 1 + (peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_STAREQUAL : TOKEN_STAR;
            return make_token(lexer, type, token_start, len);
        }
        case '!': {
            size_t len = 1 + (peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_NOTEQUAL : TOKEN_BANG;
            return make_token(lexer, type, token_start, len);
        }
        case '=': {
            size_t len = 1 + (peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_EQUALEQUAL : TOKEN_EQUALS;
            return make_token(lexer, type, token_start, len);
        }
        case '<': {
            size_t len = 1 + (peek(lexer) == '=' || peek(lexer) == '<' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_LESSEQUAL :
                            match(lexer, '<') ? TOKEN_LESSLESS : TOKEN_LESS;
            return make_token(lexer, type, token_start, len);
        }
        case '>': {
            size_t len = 1 + (peek(lexer) == '=' || peek(lexer) == '>' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_GREATEREQUAL :
                            match(lexer, '>') ? TOKEN_GREATERGREATER : TOKEN_GREATER;
            return make_token(lexer, type, token_start, len);
        }
        case '&': {
            size_t len = 1 + (peek(lexer) == '&' || peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '&') ? TOKEN_ANDAND :
                            match(lexer, '=') ? TOKEN_ANDEQUAL : TOKEN_AMPERSAND;
            return make_token(lexer, type, token_start, len);
        }
        case '|': {
            size_t len = 1 + (peek(lexer) == '|' || peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '|') ? TOKEN_OROR :
                            match(lexer, '=') ? TOKEN_OREQUAL : TOKEN_PIPE;
            return make_token(lexer, type, token_start, len);
        }
        case '^': {
            size_t len = 1 + (peek(lexer) == '=' ? 1 : 0);
            TokenType type = match(lexer, '=') ? TOKEN_XOREQUAL : TOKEN_CARET;
            return make_token(lexer, type, token_start, len);
        }
        case '?': return make_token(lexer, TOKEN_QUESTION, token_start, 1);
        case ':': return make_token(lexer, TOKEN_COLON, token_start, 1);
        case '"': return string(lexer);
    }
    
    return make_token(lexer, TOKEN_ERROR);
}

char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER_LITERAL: return "INTEGER";
        case TOKEN_FLOAT_LITERAL: return "FLOAT";
        case TOKEN_STRING_LITERAL: return "STRING";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}
