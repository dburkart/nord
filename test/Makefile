BASE = ..

include $(BASE)/Makefile.shared

all: runner lex/run parse/run compile/run vm/run
	@echo Running Test Suite...
	@echo
	@./testrunner

runner: testrunner.c
	$(CC) $(CFLAGS) testrunner.c -o testrunner

lex/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h lex/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c lex/run.c -o lex/run

parse/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h $(BASE)/src/parse.c $(BASE)/src/parse.h parse/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c $(BASE)/src/parse.c parse/run.c -o parse/run

compile/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h $(BASE)/src/parse.c $(BASE)/src/parse.h $(BASE)/src/symbol.c $(BASE)/src/compile.c $(BASE)/src/machine/disassemble.c $(BASE)/src/hash.c $(BASE)/src/machine/bytecode.c compile/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c $(BASE)/src/parse.c $(BASE)/src/symbol.c $(BASE)/src/compile.c $(BASE)/src/machine/disassemble.c $(BASE)/src/hash.c $(BASE)/src/machine/bytecode.c $(BASE)/src/machine/binary.c $(BASE)/src/machine/memory.c compile/run.c -o compile/run

vm/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h $(BASE)/src/parse.c $(BASE)/src/parse.h $(BASE)/src/symbol.c $(BASE)/src/compile.c $(BASE)/src/machine/disassemble.c $(BASE)/src/hash.c $(BASE)/src/machine/bytecode.c $(BASE)/src/machine/vm.c vm/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c $(BASE)/src/parse.c $(BASE)/src/symbol.c $(BASE)/src/compile.c $(BASE)/src/machine/disassemble.c $(BASE)/src/hash.c $(BASE)/src/machine/bytecode.c $(BASE)/src/machine/binary.c $(BASE)/src/machine/memory.c $(BASE)/src/machine/vm.c vm/run.c -o vm/run
