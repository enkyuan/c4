#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Parser rule structure
typedef struct {
    Expression* (*prefix)(Parser*, bool);
    Expression* (*infix)(Parser*, Expression*, bool);
    Precedence precedence;
} ParseRule;

// Parser implementation
Parser* parser_init(Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current = NULL;
    parser->previous = NULL;
    parser->error = NULL;
    parser->panic_mode = false;
    parser->had_error = false;
    
    // Prime the parser with the first token
    advance(parser);
    return parser;
}

void parser_free(Parser* parser) {
    if (parser->error) {
        free(parser->error->message);
        free(parser->error);
    }
    free(parser);
}

// Error handling
void parser_error_at_current(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    ParseError* error = malloc(sizeof(ParseError));
    error->message = strdup(message);
    error->token = parser->current;
    error->line = parser->current->line;
    error->column = parser->current->column;
    error->filename = parser->lexer->filename;
    
    parser->error = error;
}

void parser_error_at(Parser* parser, Token* token, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    ParseError* error = malloc(sizeof(ParseError));
    error->message = strdup(message);
    error->token = token;
    error->line = token->line;
    error->column = token->column;
    error->filename = parser->lexer->filename;
    
    parser->error = error;
}

// Token handling
Token* advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = lexer_next_token(parser->lexer);
    
    if (parser->current->type == TOKEN_ERROR) {
        parser_error_at_current(parser, "Invalid token");
    }
    
    return parser->current;
}

Token* consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current->type == type) {
        Token* token = parser->current;
        advance(parser);
        return token;
    }
    
    parser_error_at_current(parser, message);
    return NULL;
}

bool check(Parser* parser, TokenType type) {
    return parser->current->type == type;
}

bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

// Forward declarations for recursive descent
static Expression* expression(Parser* parser) {
    return parse_precedence(parser, PREC_ASSIGNMENT);
}

static Statement* statement(Parser* parser) {
    if (match(parser, TOKEN_IF)) return if_statement(parser);
    if (match(parser, TOKEN_WHILE)) return while_statement(parser);
    if (match(parser, TOKEN_FOR)) return for_statement(parser);
    if (match(parser, TOKEN_RETURN)) return return_statement(parser);
    if (match(parser, TOKEN_LBRACE)) return block_statement(parser);
    
    return expression_statement(parser);
}

static Statement* declaration(Parser* parser) {
    Statement* stmt;
    
    if (match(parser, TOKEN_VAR)) {
        stmt = var_declaration(parser);
    } else {
        stmt = statement(parser);
    }
    
    if (parser->panic_mode) parser_synchronize(parser);
    return stmt;
}

static ParseRule* get_rule(TokenType type);

// Expression parsing with precedence climbing
static Expression* binary(Parser* parser, Expression* left, bool can_assign) {
    TokenType operator_type = parser->previous->type;
    
    // Get the rule for the operator
    ParseRule* rule = get_rule(operator_type);
    Expression* right = parse_precedence(parser, (Precedence)(rule->precedence + 1));
    
    return create_binary_expr(left, right, operator_type, parser->previous);
}

static Expression* unary(Parser* parser, bool can_assign) {
    TokenType operator_type = parser->previous->type;
    
    // Parse the operand with unary precedence
    Expression* operand = parse_precedence(parser, PREC_UNARY);
    
    return create_unary_expr(operand, operator_type, true, parser->previous);
}

static Expression* grouping(Parser* parser, bool can_assign) {
    Expression* expr = expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
    return expr;
}

static Expression* number(Parser* parser, bool can_assign) {
    return create_literal_expr(parser->previous);
}

static Expression* string(Parser* parser, bool can_assign) {
    return create_literal_expr(parser->previous);
}

static Expression* variable(Parser* parser, bool can_assign) {
    return create_identifier_expr(parser->previous);
}

// Get parsing rule for token type
static ParseRule* get_rule(TokenType type) {
    static ParseRule rules[] = {
        [TOKEN_LPAREN]    = {grouping, NULL,   PREC_NONE},
        [TOKEN_MINUS]     = {unary,    binary, PREC_TERM},
        [TOKEN_PLUS]      = {NULL,     binary, PREC_TERM},
        [TOKEN_SLASH]     = {NULL,     binary, PREC_FACTOR},
        [TOKEN_STAR]      = {NULL,     binary, PREC_FACTOR},
        [TOKEN_INTEGER_LITERAL] = {number, NULL, PREC_NONE},
        [TOKEN_STRING_LITERAL] = {string, NULL, PREC_NONE},
        [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
        // Add more rules for other operators
    };
    
    return &rules[type];
}

// Parse with precedence climbing
Expression* parse_precedence(Parser* parser, Precedence precedence) {
    advance(parser);
    ParseRule* rule = get_rule(parser->previous->type);
    if (rule->prefix == NULL) {
        parser_error_at_current(parser, "Expect expression.");
        return NULL;
    }
    
    bool can_assign = precedence <= PREC_ASSIGNMENT;
    Expression* expr = rule->prefix(parser, can_assign);
    
    while (precedence <= get_rule(parser->current->type)->precedence) {
        advance(parser);
        rule = get_rule(parser->previous->type);
        expr = rule->infix(parser, expr, can_assign);
    }
    
    return expr;
}

// Top-level parsing functions
Expression* parse_expression(Parser* parser) {
    return parse_precedence(parser, PREC_ASSIGNMENT);
}

Statement* parse_declaration(Parser* parser) {
    Statement* stmt;
    
    if (match(parser, TOKEN_VAR)) {
        stmt = var_declaration(parser);
    } else {
        stmt = statement(parser);
    }
    
    if (parser->panic_mode) parser_synchronize(parser);
    return stmt;
}

Statement* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_IF)) return if_statement(parser);
    if (match(parser, TOKEN_WHILE)) return while_statement(parser);
    if (match(parser, TOKEN_FOR)) return for_statement(parser);
    if (match(parser, TOKEN_RETURN)) return return_statement(parser);
    if (match(parser, TOKEN_LBRACE)) return block_statement(parser);
    
    return expression_statement(parser);
}

Statement* parse_program(Parser* parser) {
    Statement** statements = NULL;
    int count = 0;
    int capacity = 8;
    
    statements = malloc(sizeof(Statement*) * capacity);
    
    while (!match(parser, TOKEN_EOF)) {
        if (count >= capacity) {
            capacity *= 2;
            statements = realloc(statements, sizeof(Statement*) * capacity);
        }
        
        statements[count++] = declaration(parser);
        
        if (parser->had_error) break;
    }
    
    return create_compound_stmt(statements, count, parser->previous);
}

// Error recovery
void parser_synchronize(Parser* parser) {
    parser->panic_mode = false;
    
    while (parser->current->type != TOKEN_EOF) {
        if (parser->previous->type == TOKEN_SEMICOLON) return;
        
        switch (parser->current->type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
                return;
            default:
                ; // Do nothing
        }
        
        advance(parser);
    }
}
// Statement parsing functions
Statement* if_statement(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'");
    Expression* condition = expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after if condition");

    Statement* then_branch = statement(parser);
    Statement* else_branch = NULL;
    if (match(parser, TOKEN_ELSE)) {
        else_branch = statement(parser);
    }

    return create_if_stmt(condition, then_branch, else_branch, parser->previous);
}

Statement* while_statement(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'");
    Expression* condition = expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after while condition");

    Statement* body = statement(parser);
    return create_while_stmt(condition, body, parser->previous);
}

Statement* for_statement(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'for'");

    Statement* initializer;
    if (match(parser, TOKEN_SEMICOLON)) {
        initializer = NULL;
    } else if (match(parser, TOKEN_VAR)) {
        initializer = var_declaration(parser);
    } else {
        initializer = expression_statement(parser);
    }

    Expression* condition = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        condition = expression(parser);
    }
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition");

    Statement* increment = NULL;
    if (!check(parser, TOKEN_RPAREN)) {
        Expression* increment_expr = expression(parser);
        increment = create_expression_stmt(increment_expr, parser->previous);
    }
    consume(parser, TOKEN_RPAREN, "Expect ')' after for clauses");

    Statement* body = statement(parser);
    return create_for_stmt(initializer, condition, increment, body, parser->previous);
}

Statement* return_statement(Parser* parser) {
    Token* keyword = parser->previous;
    Expression* value = NULL;

    if (!check(parser, TOKEN_SEMICOLON)) {
        value = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value");
    return create_return_stmt(value, keyword);
}

Statement* block_statement(Parser* parser) {
    Statement** statements = NULL;
    int count = 0;
    int capacity = 8;

    statements = malloc(sizeof(Statement*) * capacity);

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        if (count >= capacity) {
            capacity *= 2;
            statements = realloc(statements, sizeof(Statement*) * capacity);
        }

        statements[count++] = declaration(parser);
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after block");
    return create_compound_stmt(statements, count, parser->previous);
}

Statement* expression_statement(Parser* parser) {
    Expression* expr = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression");
    return create_expression_stmt(expr, parser->previous);
}

Statement* var_declaration(Parser* parser) {
    Token* name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name");

    Expression* initializer = NULL;
    if (match(parser, TOKEN_EQUALS)) {
        initializer = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration");
    return create_var_stmt(name, initializer, parser->previous);
}