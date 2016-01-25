/* Generated file, don't edit! */
%%{

machine word;

include latin "latin.rl";
include fr_prefix "fr_prefix.rl";

# Elisions:
#
#     d', l', qu', y', etc.

prefix = fr_prefix apostrophe;

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
# The word pattern overlaps with the abbreviation pattern, and must then be
# placed below it in the scanner definition or abbreviations would not be
# recognized as such.
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

word_bracket = "(" latin_letter latin* ")" "-"? latin+ word_tail
             | latin+ word_chunk* "-"? "(" latin_letter latin* ")" ("-"? latin+ word_tail)?
             ;

# Word containing double quotes. Mostly used in German:
#
#    "New-Age"-Ideologie
#    "KwaMadala"-Hostels
#    "Pferde"-Kavallerie

word_double_quote = opening_double_quote
                    latin+ word_chunk*
                    closing_double_quote
                    "-" latin+ word_chunk*;

# Substraction needed below for correct tokenization of prefixes.

word = (word_normal | word_bracket | word_double_quote)
     - (prefix word_normal) - (prefix word_bracket) - (prefix word_double_quote)
     ;

}%%
