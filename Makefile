CC = gcc
CFLAGS = -Wall -Werror

all: main

main: main.o lexer.o parser.o
    $(CC) $(CFLAGS) -o $@ $^

main.o: main.c lexer.h parser.h
lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h

clean:
    rm -f main *.o

