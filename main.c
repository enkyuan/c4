#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read entire source file into memory
static char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file '%s'\n", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    // Allocate buffer and read file
    char* buffer = malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read file '%s'\n", filename);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read < size) {
        fprintf(stderr, "Could not read file '%s'\n", filename);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source>\n", argv[0]);
        return 1;
    }

    // Read source file
    char* source = read_file(argv[1]);
    if (source == NULL) return 1;

    // Initialize compiler components
    Lexer* lexer = lexer_init(source, argv[1]);
    Parser* parser = parser_init(lexer);
    SemanticAnalyzer* analyzer = semantic_init();
    analyzer->filename = strdup(argv[1]);

    // Parse program
    Statement* program = parse_program(parser);
    if (parser->had_error) {
        fprintf(stderr, "%s:%d:%d: %s\n",
                parser->error->filename,
                parser->error->line,
                parser->error->column,
                parser->error->message);
        goto cleanup;
    }

    // Perform semantic analysis
    check_statement(analyzer, program);
    if (analyzer->had_error) {
        goto cleanup;
    }

    // Generate code
    char* output_file = "output.s";
    FILE* output = fopen(output_file, "w");
    if (output == NULL) {
        fprintf(stderr, "Could not create output file '%s'\n", output_file);
        goto cleanup;
    }

    CodeGenerator* gen = codegen_init(output, true);
    generate_program(gen, program);
    fclose(output);

    // Cleanup
cleanup:
    free_statement(program);
    semantic_free(analyzer);
    parser_free(parser);
    lexer_free(lexer);
    free(source);

    return 0;
}
