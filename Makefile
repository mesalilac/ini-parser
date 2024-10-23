CFLAGS=-Wall -Wextra -Wformat -pedantic
LIBS=
IDIR=src/include
DEPS=$(IDIR)/ini.c
OUTPUT=a.out

build:
	cc $(CFLAGS) src/main.c -o $(OUTPUT) $(DEPS) $(LIBS)

run: build
	./$(OUTPUT)
