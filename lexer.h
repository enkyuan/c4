#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// Token types for C11 language features
typedef enum {
    // Keywords
    TOKEN_AUTO, TOKEN_BREAK, TOKEN_CASE, TOKEN_CHAR,
    TOKEN_CONST, TOKEN_CONTINUE, TOKEN_DEFAULT, TOKEN_DO,
    TOKEN_DOUBLE, TOKEN_ELSE, TOKEN_ENUM, TOKEN_EXTERN,
    TOKEN_FLOAT, TOKEN_FOR, TOKEN_GOTO, TOKEN_IF,
    TOKEN_INLINE, TOKEN_INT, TOKEN_LONG, TOKEN_REGISTER,
    TOKEN_RESTRICT, TOKEN_RETURN, TOKEN_SHORT, TOKEN_SIGNED,
    TOKEN_SIZEOF, TOKEN_STATIC, TOKEN_STRUCT, TOKEN_SWITCH,
    TOKEN_TYPEDEF, TOKEN_UNION, TOKEN_UNSIGNED, TOKEN_VOID,
    TOKEN_VOLATILE, TOKEN_WHILE,
    TOKEN_CLASS, TOKEN_FUN, TOKEN_VAR,  // Additional keywords

    // Single-character tokens
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_SEMICOLON, TOKEN_COMMA,
    TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_BANG, TOKEN_EQUALS, TOKEN_LESS, TOKEN_GREATER,
    TOKEN_AMPERSAND, TOKEN_PIPE, TOKEN_CARET, TOKEN_QUESTION,
    TOKEN_COLON,

    // Two-character tokens
    TOKEN_MINUSEQUAL, TOKEN_MINUSMINUS, TOKEN_PLUSPLUS,
    TOKEN_PLUSEQUAL, TOKEN_SLASHEQUAL, TOKEN_STAREQUAL,
    TOKEN_NOTEQUAL, TOKEN_EQUALEQUAL, TOKEN_LESSEQUAL,
    TOKEN_LESSLESS, TOKEN_GREATEREQUAL, TOKEN_GREATERGREATER,
    TOKEN_ANDAND, TOKEN_ANDEQUAL, TOKEN_OROR, TOKEN_OREQUAL,
    TOKEN_XOREQUAL,

    // Literals
    TOKEN_IDENTIFIER, TOKEN_INTEGER_LITERAL, TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,

    // Special tokens
    TOKEN_ERROR, TOKEN_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
    union {
        long long int_value;
        double float_value;
        char* string_value;
    } value;
} Token;

// Lexer structure
typedef struct {
    char* source;
    size_t source_length;
    size_t current;
    int line;
    int column;
    char* filename;
} Lexer;

// Lexer interface functions
Lexer* lexer_init(char* source, char* filename);
void lexer_free(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
char* token_type_to_string(TokenType type);

#endif // LEXER_H