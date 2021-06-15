BASE=.

include Makefile.shared

BINARY=nord

.PHONY: test $(BINARY)

all: $(OBJECTS) $(BINARY)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BINARY):
	$(CC) -c $(CFLAGS) $(BASE)/src/main.c -o $(BASE)/src/main.o
	$(CC) $(CFLAGS) -o $(BINARY) $(OBJECTS) $(BASE)/src/main.o

test: $(OBJECTS)
	make -C test

clean:
	make -C test clean
	rm nord
	rm $(OBJECTS)
