RAGEL_CFLAGS = -I./fsm -I./gen -e -T1

UNI  = gen/combining_diacritic.rl gen/raw_latin_letter.rl \
       gen/raw_latin_uppercase.rl gen/raw_latin_lowercase.rl
LANG = gen/en_prefix.rl gen/en_suffix.rl gen/en_lexicon.rl \
       gen/fr_prefix.rl gen/fr_suffix.rl gen/fr_lexicon.rl \
       gen/it_prefix.rl gen/it_suffix.rl gen/it_lexicon.rl \
       gen/generic_prefix.rl gen/generic_suffix.rl gen/generic_lexicon.rl
TPL  = gen/en_tokenize.rl gen/en_suffix_match.rl gen/en_word.rl \
       gen/fr_tokenize.rl gen/fr_suffix_match.rl gen/fr_word.rl \
       gen/it_tokenize.rl gen/it_suffix_match.rl gen/it_word.rl \
       gen/generic_tokenize.rl gen/generic_suffix_match.rl gen/generic_word.rl
ABR  = gen/abbr_lexicon.rl
CODE = gen/en_tokenize.ic gen/en_suffix_match.ic \
       gen/fr_tokenize.ic gen/fr_suffix_match.ic \
       gen/it_tokenize.ic gen/it_suffix_match.ic \
       gen/generic_tokenize.ic gen/generic_suffix_match.ic \
       gen/sentencize.ic

REST = $(wildcard fsm/*.rl)

all: $(UNI) $(LANG) $(TPL) $(ABR) $(CODE)

clean:
	rm -f gen/*

gen/%_tokenize.ic: gen/%_tokenize.rl $(UNI) $(REST) gen/%_prefix.rl \
                   gen/%_suffix.rl gen/%_lexicon.rl gen/%_word.rl
	ragel $(RAGEL_CFLAGS) $< -o $@

gen/%_suffix_match.ic: gen/%_suffix_match.rl scripts/reverse_machine.py
	ragel $(RAGEL_CFLAGS) $< -o $@
	scripts/reverse_machine.py $@

gen/sentencize.ic: fsm/sentencize.rl $(UNI) $(REST)
	ragel $(RAGEL_CFLAGS) $< -o $@

$(UNI): scripts/mkunicode.py
	$< $(notdir $(basename $@)) > $@

$(ABR): data/abbr_lexicon.txt scripts/mkabbrs.py
	scripts/mkabbrs.py < $< > $@

gen/%_tokenize.rl: scripts/mktpl.py fsm/tokenize.tpl
	$< $(notdir $(basename $@)) > $@

gen/%_suffix_match.rl: scripts/mktpl.py fsm/suffix_match.tpl
	$< $(notdir $(basename $@)) > $@

gen/%_word.rl: scripts/mktpl.py fsm/word.tpl
	$< $(notdir $(basename $@)) > $@

gen/%.rl: data/%.txt
	scripts/mklang.py $^ > $@

.PHONY: all clean
