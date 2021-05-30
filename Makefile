CC=clang
CFLAGS=-Werror -std=c99

SOURCES=src/token.c src/lexer.c src/main.c
BINARY=nord

all: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BINARY)
