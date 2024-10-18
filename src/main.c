// TODO: fix type passing error with file pointer - line 22

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

  initLexer(input_file);
  initParser();
  initSemantic();
  initCodegen();

  while (1) {
    Token token = getNextToken();
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

