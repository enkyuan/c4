// TODO: expand on boilerplate code

#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"
#include "error.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  FILE *input_file = fopen(argv[1], "r");
  if (input_file == NULL) {
    fprintf(stderr, "Error: could not open input file %s\n", argv[1]);
    return 1;
  }

  lexer_init(input_file);
  parser_init();
  semantic_init();
  codegen_init();

  while (1) {
    Token token = lexer_get_next_token();
    if (token.type == TOKEN_EOF) {
      break;
    }
    parser_parse_token(token);
  }

  semantic_analyze();
  codegen_generate();

  fclose(input_file);
  return 0;
}

