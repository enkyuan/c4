#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    ERR_NONE,       
    ERR_SYNTAX,     
    ERR_UNEXPECTED_EOF, 
    ERR_UNKNOWN_TOKEN, 
    ERR_UNDECLARED_IDENTIFIER, 
    ERR_TYPE_MISMATCH, 
    ERR_DIVIDE_BY_ZERO, 
    ERR_INTERNAL,      
} ErrorCode;

void reportError(ErrorCode code, const char *message);

void handleFatalError(ErrorCode code, const char *message);

#endif // ERROR_H
