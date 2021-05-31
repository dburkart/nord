BASE = ..

include $(BASE)/Makefile.shared

all: lexer/run
	python3 -m unittest discover -s . -p '*_test.py'

lexer/run: $(BASE)/src/lexer.c $(BASE)/src/lexer.h $(BASE)/src/token.c $(BASE)/src/token.h lexer/run.c
	$(CC) $(CFLAGS) -I$(BASE)/src $(BASE)/src/lexer.c $(BASE)/src/token.c lexer/run.c -o lexer/run
