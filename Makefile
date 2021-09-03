BASE=.

include Makefile.shared

BINARIES=nord

.PHONY: test $(BINARY)

all: $(OBJECTS) $(BINARIES)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BINARIES):
	$(CC) -c $(CFLAGS) $(BASE)/src/binaries/$@.c -o $(BASE)/src/binaries/$@.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(BASE)/src/binaries/$@.o

$(BINARY):
	$(CC) -c $(CFLAGS) $(BASE)/src/main.c -o $(BASE)/src/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BINARY) $(OBJECTS) $(BASE)/src/main.o

test: $(OBJECTS)
	make -C test

clean:
	make -C test clean
	rm -f $(BINARIES)
	rm -f src/main.o
	rm -f $(OBJECTS)
