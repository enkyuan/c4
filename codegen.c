#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Code generator initialization
CodeGenerator* codegen_init(FILE* output, bool optimize) {
    CodeGenerator* gen = malloc(sizeof(CodeGenerator));
    gen->output = output;
    gen->blocks = NULL;
    gen->block_count = 0;
    gen->live_ranges = NULL;
    gen->current_stack_offset = 0;
    gen->label_counter = 0;
    gen->optimize = optimize;
    
    // Initialize registers
    for (int i = 0; i < 16; i++) {
        gen->registers[i].is_available = true;
        gen->registers[i].current_var = NULL;
        gen->registers[i].is_dirty = false;
    }
    
    return gen;
}

void codegen_free(CodeGenerator* gen) {
    // Free basic blocks
    for (int i = 0; i < gen->block_count; i++) {
        free(gen->blocks[i]);
    }
    free(gen->blocks);
    
    // Free live ranges
    LiveRange* current = gen->live_ranges;
    while (current != NULL) {
        LiveRange* next = current->next;
        free(current->var_name);
        free(current);
        current = next;
    }
    
    free(gen);
}

// Code generation functions
void generate_program(CodeGenerator* gen, Statement* program) {
    // Generate assembly prologue
    fprintf(gen->output, ".text\n");
    fprintf(gen->output, ".globl _main\n");
    fprintf(gen->output, "_main:\n");
    
    // Set up stack frame
    fprintf(gen->output, "    push {fp, lr}\n");
    fprintf(gen->output, "    mov fp, sp\n");
    
    // Generate code for the program
    if (program->type == NODE_COMPOUND) {
        for (int i = 0; i < program->as.compound.count; i++) {
            generate_statement(gen, program->as.compound.statements[i]);
        }
    } else {
        generate_statement(gen, program);
    }
    
    // Generate assembly epilogue
    fprintf(gen->output, "    mov sp, fp\n");
    fprintf(gen->output, "    pop {fp, pc}\n");
}

void generate_statement(CodeGenerator* gen, Statement* stmt) {
    switch (stmt->type) {
        case NODE_EXPRESSION:
            generate_expression(gen, stmt->as.expression.expr);
            break;
        case NODE_RETURN:
            if (stmt->as.return_stmt.value) {
                generate_expression(gen, stmt->as.return_stmt.value);
                fprintf(gen->output, "    mov r0, r1\n"); // Move return value to r0
            }
            break;
        case NODE_DECLARATION:
            // Handle variable declarations
            break;
        default:
            break;
    }
}

void generate_expression(CodeGenerator* gen, Expression* expr) {
    switch (expr->type) {
        case NODE_LITERAL:
            if (expr->token->type == TOKEN_INTEGER_LITERAL) {
                fprintf(gen->output, "    mov r1, #%s\n", expr->token->lexeme);
            }
            break;
        case NODE_BINARY_OP:
            generate_expression(gen, expr->as.binary.left);
            // Save left result
            fprintf(gen->output, "    push {r1}\n");
            generate_expression(gen, expr->as.binary.right);
            // Restore left result to r2
            fprintf(gen->output, "    pop {r2}\n");
            
            // Perform operation
            switch (expr->token->type) {
                case TOKEN_PLUS:
                    fprintf(gen->output, "    add r1, r2, r1\n");
                    break;
                case TOKEN_MINUS:
                    fprintf(gen->output, "    sub r1, r2, r1\n");
                    break;
                case TOKEN_STAR:
                    fprintf(gen->output, "    mul r1, r2, r1\n");
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}