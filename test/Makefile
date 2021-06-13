BASE = ..

include $(BASE)/Makefile.shared

all: runner lex/run parse/run compile/run vm/run
	@echo Running Test Suite...
	@echo
	@./testrunner

runner: testrunner.c
	$(CC) $(CFLAGS) -I$(BASE)/src testrunner.c -o testrunner

lex/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

parse/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

compile/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

vm/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

clean:
	rm lex/run
	rm parse/run
	rm compile/run
	rm vm/run
