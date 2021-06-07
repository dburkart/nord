include Makefile.shared

SOURCES=src/token.c src/lex.c src/parse.c src/bytecode.c src/vm.c src/main.c
BINARY=nord

.PHONY: test

all: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BINARY)

test:
	make -C test
