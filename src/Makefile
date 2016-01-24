RAGEL_CFLAGS = -I. -I./gen -e -T1

UNI  = gen/combining_diacritic.rl gen/raw_latin_letter.rl \
       gen/raw_latin_uppercase.rl
LANG = gen/en_prefix.rl gen/en_suffix.rl gen/en_lexicon.rl \
       gen/fr_prefix.rl gen/fr_suffix.rl gen/fr_lexicon.rl \
       gen/it_prefix.rl gen/it_suffix.rl gen/it_lexicon.rl
TPL  = gen/en_tokenize.rl gen/en_suffix_match.rl gen/en_word.rl \
       gen/fr_tokenize.rl gen/fr_suffix_match.rl gen/fr_word.rl \
       gen/it_tokenize.rl gen/it_suffix_match.rl gen/it_word.rl
ABR  = gen/abbr_lexicon.rl
CODE = gen/en_tokenize.ic gen/en_suffix_match.ic \
       gen/fr_tokenize.ic gen/fr_suffix_match.ic \
       gen/it_tokenize.ic gen/it_suffix_match.ic \
       gen/sentencize.ic

REST = $(wildcard *.rl)

all: $(UNI) $(LANG) $(TPL) $(ABR) $(CODE)

clean:
	rm -f gen/*

gen/%_tokenize.ic: gen/%_tokenize.rl $(UNI) $(REST) gen/%_prefix.rl \
                   gen/%_suffix.rl gen/%_lexicon.rl gen/%_word.rl
	ragel $(RAGEL_CFLAGS) $< -o $@

gen/%_suffix_match.ic: gen/%_suffix_match.rl
	ragel $(RAGEL_CFLAGS) $< -o $@
	./reverse_machine.py $@

gen/sentencize.ic: sentencize.rl $(UNI) $(REST)
	ragel $(RAGEL_CFLAGS) $< -o $@

$(UNI): mkunicode.py
	./$< $(notdir $(basename $@)) > $@

$(ABR): mkabbrs.py
	./$< < $(notdir $(basename $@)).txt > $@

gen/%_tokenize.rl: mktpl.py tokenize.tpl
	./$< $(notdir $(basename $@)) > $@

gen/%_suffix_match.rl: mktpl.py suffix_match.tpl
	./$< $(notdir $(basename $@)) > $@

gen/%_word.rl: mktpl.py word.tpl
	./$< $(notdir $(basename $@)) > $@

gen/%.rl: mklang.py %.txt
	./$< $(notdir $(basename $@)) > $@

.PHONY: all clean
