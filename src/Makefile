CC = gcc
CFLAGS = -Wall -Werror

.PHONY: all clean

all: main

main: main.o lexer.o parser.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c lexer.h parser.h
	$(CC) $(CFLAGS) -c $< -o $@

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c $< -o $@

# If you have these additional dependencies, uncomment and adjust as needed
# error.o: error.c error.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# codegen.o: codegen.c codegen.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# semantic.o: semantic.c semantic.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# ast.o: ast.c ast.h
# 	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
