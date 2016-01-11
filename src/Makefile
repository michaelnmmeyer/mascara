MACHINES = gen/tokenize_en.ic gen/tokenize_fr.ic gen/suffix_en.ic gen/suffix_fr.ic

all: $(MACHINES)

gen/tokenize_%.ic: gen/tokenize_%.rl
	ragel -I. -I./gen -e -T1 $< -o $@

gen/suffix_%.ic: gen/suffix_%.rl
	ragel -I. -I./gen -e -T1 $< -o $@
	./reverse_machine.py $@

.PHONY: all
