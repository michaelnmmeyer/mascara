PREFIX = /usr/local

CFLAGS = -std=c11 -g -Wall -Werror
CFLAGS += -O2 -DNDEBUG -march=native -mtune=native -fomit-frame-pointer -s

AMALG = mascara.h mascara.c

#--------------------------------------
# Abstract targets
#--------------------------------------

all: $(AMALG) mascara example

clean:
	rm -f mascara test/mascara.so vgcore* core

realclean: clean
	rm -f cmd/*.ih src/gen/*

check: test/mascara.so
	cd test && valgrind --leak-check=full --error-exitcode=1 lua test.lua

install: mascara
	install -spm 0755 $< $(PREFIX)/bin/mascara

uninstall:
	rm -f $(PREFIX)/bin/mascara

.PHONY: all clean realclean check install uninstall


#--------------------------------------
# Concrete targets
#--------------------------------------

cmd/%.ih: cmd/%.txt
	cmd/mkcstring.py < $< > $@

mascara.h: src/api.h
	cp $< $@

mascara.c: $(wildcard src/*.h src/*.c src/gen/*.ic)
	src/mkamalg.py src/*.c > $@

mascara: $(AMALG) cmd/mascara.ih cmd/mascara.c cmd/cmd.c
	$(CC) $(CFLAGS) mascara.c cmd/mascara.c cmd/cmd.c -o $@

example: example.c $(AMALG)
	$(CC) $(CFLAGS) $< mascara.c -o $@

test/mascara.so: $(AMALG) test/mascara.c
	$(CC) $(CFLAGS) -fPIC -shared mascara.c test/mascara.c -o $@
