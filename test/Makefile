BASE = ..

include $(BASE)/Makefile.shared

all: runner lex/run parse/run compile/run vm/run interpret/run
	@echo Running Test Suite...
	@echo
	@./testrunner

runner: testrunner.c
	$(CC) $(CFLAGS) -I$(BASE)/src testrunner.c -o testrunner

lex/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

parse/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

compile/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

vm/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

interpret/run:
	$(CC) -c $(CFLAGS) -I$(BASE)/src $@.c -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(BASE)/src -o $@ $(OBJECTS) $@.o

clean:
	rm -f */run */*.o
	rm -f testrunner
