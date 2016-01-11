/* Generated file, don't edit! */
%%{

machine word;

include latin "latin.rl";
include en_prefix "en_prefix.rl";

# We don't consider "_" to be a word character. Never used like that in natural
# language.
#
# The apostrophe is needed to match words like:
#
#    entr'ouvert
#    aujourd'hui
#    shi'isme
#    tape-à-l'œil
#
# The addition of ² and ³ is needed to match km², m³, etc. We used a specific
# pattern for units before, but it turns out that these characters are also used
# in regular words, e.g.:
#
#    ENSE³   École nationale supérieure de l'énergie, l'eau et l'environnement
#
# Besides, there is no good reason to separate these characters from a word
# if they follow it immediately.
#
# This pattern also matches abbreviations:
#
#    A.B.C.
#
# I don't know if it would be beneficial to use a separate pattern for them.
# Which words other than abbreviation have internal periods? Websites?
#
# If the word is an abbreviation, we detach the terminal period temporarily. It
# is the job of the sentence boundary detection module to decide what to do with
# a trailing period.
#
# The & is there to match words like:
#
#     AT&T
#     R&B-based
#     R&B-inflected
#
# Didn't find a single word that contains several &. We still allow several
# internal &.
#
# The ° is there to match words like:
#
#     nirvāṇa°
#     °C
#
# Some words start with a number, so we don't constrain words to start with a
# letter:
#
#     2-Amino-5-phosphonovaleriansäure
#
# We don't consider / as a word character, because it is generally used to
# separate discrete words. Sometimes it isn't, but this usage is wrong anyway:
#
#    restaurateur/trice

# Elisions:
#
#     d', l', qu', y', etc.

elision = en_prefix apostrophe;

word_chunk = (apostrophe | "-" | "." | "&") latin+;

word_tail = word_chunk* ("²" | "³" | "°")?;

word_normal = "°"? latin+ word_tail;

# Word containing internal brackets.
#
#     (pré)opératoire
#     molybdène(III)
#     (MIP)-1alpha
#     désir(s)
#     4-(parahydroxyphenyl)-2-butanone

word_bracket = "(" latin_letter+ ")" "-"? latin+ word_tail
             | latin+ word_chunk* "-"? "(" latin_letter+ ")" ("-"? latin+ word_tail)?
             ;

# Substraction needed below for correct tokenization of elisions.

word = (word_normal | word_bracket) - (elision word_normal) - (elision word_bracket);

}%%
