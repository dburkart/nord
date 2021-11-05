BASE=.

include Makefile.shared

BINARIES=nord

.PHONY: test

all: $(OBJECTS) $(BINARIES)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BINARIES): $(OBJECTS)
	$(CC) -c $(CFLAGS) $(BASE)/src/binaries/$@.c -o $(BASE)/src/binaries/$@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(BASE)/src/binaries/$@.o

test: $(OBJECTS)
	make -C test

clean:
	make -C test clean
	rm -f $(BINARIES)
	rm -f src/main.o
	rm -f $(OBJECTS)
