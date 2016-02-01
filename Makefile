PREFIX = /usr/local

CFLAGS = -std=c11 -g -Wall -Werror
CFLAGS += -O2 -DNDEBUG -march=native -mtune=native -fomit-frame-pointer -s
CFLAGS += -flto

AMALG = mascara.h mascara.c

EXAMPLES = $(patsubst %.c,%,$(wildcard examples/*))

#--------------------------------------
# Abstract targets
#--------------------------------------

all: $(AMALG) mascara $(EXAMPLES)

clean:
	rm -f mascara $(EXAMPLES) test/mascara.so vgcore* core

check: test/mascara.so
	cd test && valgrind --leak-check=full --error-exitcode=1 lua test.lua

install: mascara
	install -spm 0755 $< $(PREFIX)/bin/mascara

uninstall:
	rm -f $(PREFIX)/bin/mascara

.PHONY: all clean check install uninstall


#--------------------------------------
# Concrete targets
#--------------------------------------

cmd/%.ih: cmd/%.txt
	cmd/mkcstring.py < $< > $@

cmd/%.ic: cmd/%.rl
	ragel -e -T1 $^ -o $@

mascara.h: src/api.h
	cp $< $@

mascara.c: $(wildcard src/*.h src/*.c src/gen/*.ic)
	src/scripts/mkamalg.py src/*.c > $@

mascara: $(AMALG) cmd/mascara.ih cmd/print_str.ic cmd/mascara.c cmd/cmd.c
	$(CC) $(CFLAGS) mascara.c cmd/mascara.c src/lib/utf8proc.c cmd/cmd.c -o $@

examples/%: examples/%.c $(AMALG)
	$(CC) $(CFLAGS) $< mascara.c src/lib/utf8proc.c -o $@

test/mascara.so: test/mascara.c $(AMALG)
	$(CC) $(CFLAGS) -fPIC -shared $< mascara.c src/lib/utf8proc.c -o $@
