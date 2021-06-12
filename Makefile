include Makefile.shared

SOURCES=src/token.c src/lex.c src/parse.c src/machine/bytecode.c src/hash.c \
		src/symbol.c src/compile.c src/machine/memory.c src/machine/vm.c \
		src/machine/disassemble.c src/machine/binary.c src/main.c

BINARY=nord

.PHONY: test

all: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BINARY)

test:
	make -C test
