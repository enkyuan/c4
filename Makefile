CC = gcc
CFLAGS = -Wall -Werror

all: main

main: main.o lexer.o parser.o
    $(CC) $(CFLAGS) -o $@ $^

main.o: main.c lexer.h parser.h
lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h
error.o: error.c error.h
codegen.o: codegen.c codegen.h
semantic.o: semantic.c semantic.h
ast.o: ast.c ast.h

clean:
    rm -f main *.o

