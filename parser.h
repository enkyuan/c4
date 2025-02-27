#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// Precedence levels for operator parsing
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,         // ||
    PREC_AND,        // &&
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . () []
    PREC_PRIMARY
} Precedence;

// Error handling structure
typedef struct {
    char* message;
    Token* token;
    int line;
    int column;
    char* filename;
} ParseError;

// Parser state structure
typedef struct {
    Lexer* lexer;
    Token* current;
    Token* previous;
    ParseError* error;
    bool panic_mode;
    bool had_error;
} Parser;

// Parser interface functions
Parser* parser_init(Lexer* lexer);
void parser_free(Parser* parser);

// Main parsing functions
Statement* parse_program(Parser* parser);
Statement* parse_declaration(Parser* parser);
Statement* parse_statement(Parser* parser);
Expression* parse_expression(Parser* parser);

// Expression parsing with precedence climbing
Expression* parse_precedence(Parser* parser, Precedence precedence);
Expression* parse_unary(Parser* parser);
Expression* parse_primary(Parser* parser);

// Statement parsing functions
Statement* if_statement(Parser* parser);
Statement* while_statement(Parser* parser);
Statement* for_statement(Parser* parser);
Statement* return_statement(Parser* parser);
Statement* block_statement(Parser* parser);
Statement* expression_statement(Parser* parser);
Statement* var_declaration(Parser* parser);

// Error handling and recovery
void parser_error_at_current(Parser* parser, const char* message);
void parser_error_at(Parser* parser, Token* token, const char* message);
void parser_synchronize(Parser* parser);

// Helper functions
bool match(Parser* parser, TokenType type);
bool check(Parser* parser, TokenType type);
Token* advance(Parser* parser);
Token* consume(Parser* parser, TokenType type, const char* message);

#endif // PARSER_H