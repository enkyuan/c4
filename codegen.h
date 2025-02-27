#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include <stdbool.h>

// Basic block structure for control flow analysis
typedef struct BasicBlock {
    int id;
    Statement** statements;
    int statement_count;
    struct BasicBlock** predecessors;
    int predecessor_count;
    struct BasicBlock** successors;
    int successor_count;
    bool is_entry;
    bool is_exit;
} BasicBlock;

// Live range structure for register allocation
typedef struct LiveRange {
    char* var_name;
    int start_block;
    int end_block;
    int reg;
    struct LiveRange* next;
} LiveRange;

// Register descriptor
typedef struct {
    bool is_available;
    char* current_var;
    bool is_dirty;
} Register;

// Code generator state
typedef struct {
    FILE* output;
    BasicBlock** blocks;
    int block_count;
    LiveRange* live_ranges;
    Register registers[16];  // x86_64 has 16 general purpose registers
    int current_stack_offset;
    int label_counter;
    bool optimize;
} CodeGenerator;

// Code generator interface functions
CodeGenerator* codegen_init(FILE* output, bool optimize);
void codegen_free(CodeGenerator* gen);

// Basic block analysis
void build_basic_blocks(CodeGenerator* gen, Statement* program);
void analyze_control_flow(CodeGenerator* gen);

// Register allocation
void compute_live_ranges(CodeGenerator* gen);
void allocate_registers(CodeGenerator* gen);
int get_register(CodeGenerator* gen, char* var_name);
void free_register(CodeGenerator* gen, int reg);

// Code generation
void generate_program(CodeGenerator* gen, Statement* program);
void generate_function(CodeGenerator* gen, Statement* func_def);
void generate_statement(CodeGenerator* gen, Statement* stmt);
void generate_expression(CodeGenerator* gen, Expression* expr);

// Optimization functions
void optimize_basic_blocks(CodeGenerator* gen);
void eliminate_dead_code(CodeGenerator* gen);
void peephole_optimization(CodeGenerator* gen);

// Assembly generation helpers
void emit_prologue(CodeGenerator* gen);
void emit_epilogue(CodeGenerator* gen);
void emit_instruction(CodeGenerator* gen, const char* format, ...);

#endif // CODEGEN_H