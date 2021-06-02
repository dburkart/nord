BASE = ..

include $(BASE)/Makefile.shared

all: lex/run parse/run
	python3 -m unittest discover -s . -p '*_test.py'

lex/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h lex/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c lex/run.c -o lex/run

parse/run: $(BASE)/src/lex.c $(BASE)/src/lex.h $(BASE)/src/token.c $(BASE)/src/token.h $(BASE)/src/parse.c $(BASE)/src/parse.h parse/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lex.c $(BASE)/src/token.c $(BASE)/src/parse.c parse/run.c -o parse/run
