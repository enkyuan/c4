CC = gcc
CFLAGS = -Wall -Werror

OBJS = main.o lexer.o parser.o semantic.o ast.o codegen.o

main: $(OBJS)
	$(CC) $(CFLAGS) -o main $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main $(OBJS)
