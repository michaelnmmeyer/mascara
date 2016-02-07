%%{

machine sentencize;

alphtype unsigned char;

include whitespace "whitespace.rl";
include misc "misc.rl";
include abbr_lexicon "abbr_lexicon.rl";
include symbol "symbol.rl";


#-------------------------------------------------------------------------------
# Character classes.
#-------------------------------------------------------------------------------

eos_marker = period | ellipsis | "?"+ | "!"+;

# Symbols that can appear at the end of a sentence, after the terminal period.
# Only useful for English. In French, these should appear _before_ the
# terminal period.
#
sent_trail = closing_bracket | closing_single_quote | closing_double_quote;

sent_lead = opening_bracket | opening_single_quote | opening_double_quote;

#-------------------------------------------------------------------------------
# Where _not_ to split.
#-------------------------------------------------------------------------------

# Alphanumeric containing internal periods. Includes abbreviations.
#
#    Ph.D.
#    1.2.c
#    e.g.
#    i.e.
#
thing_with_periods = latin+ (period latin+)+ period?;

# A token followed by a period followed by a one of these characters is a
# likely abbreviation.
#
period_punct = latin+ period sent_trail* ("," | ";" | ":");

# Example:
#
#    John J. Doe.
#
name_initial = latin_uppercase period whitespace latin_uppercase;

# Examples:
#
#    cf.
#    bzw.
#    p. 5
#    pp. 6-7
#
# We could add consonants containing diacritics, but this is unlikely to be
# helpful. Adding uppercase consonants is not a good idea.
#
abbreviation_consonant = [bcdfghjklmnpqrstvwxz]+ period;

# Discard when followed by a lowercase letter, including exclamations:
#
#    Ah! princesse.
#    "Good gracious!" cried Mrs. Bennet.
#     "Why?" said Albert
#
no_capital = eos_marker sent_trail* whitespace* sent_lead* latin_lowercase;

not_eos = thing_with_periods
        | period_punct
        | name_initial
        | no_capital
        | abbr_lexicon
        | abbreviation_consonant
        | email | uri
        ;

# Discard when a terminator is followed by a closing bracket or closing quote
# and then another terminator:
#
#    (und vor Gelons Tod 216 v. Chr.). Er widerlegte
#
lookahead = eos_marker sent_trail* (period | "?" | "!");
   

#-------------------------------------------------------------------------------
# Main.
#-------------------------------------------------------------------------------

find_eos := |*
   period sent_trail* => {
      *period = ts;
      goto found;
   };
   eos_marker sent_trail* => {
      *period = NULL;
      goto found;
   };
   lookahead => {
      /* Drop one code point. */
      do te--;
      while ((*te & 0xc0) == 0x80);
      fexec te;
   };
   paragraph_break => {
      /* Don't include the paragraph break itself in the sentence. */
      te = ts;
      *period = NULL;
      goto found;
   };
   any;
   latin_letter+;
   not_eos;
*|;

main := whitespace* %{ start = fpc; }
        (code_point - whitespace) @{ fexec start; fcall find_eos; };

}%%

%% write data noerror nofinal;

static const unsigned char *next_sentence(struct sentencizer *tkr,
                                          size_t *len,
                                          const unsigned char **period)
{
   int cs, act, top, stack[1];
   const unsigned char *ts, *te;
   const unsigned char *p = tkr->p;
   const unsigned char *pe = tkr->pe;
   const unsigned char *const eof = pe;

   const unsigned char *start = NULL;

   %% write init;
   %% write exec;

   *period = NULL;

   /* Last sentence. Don't know how to trim whitespace on the right. */
   if (start) {
      te = eof;
      goto found;
   }
   return NULL;

found:
   *len = te - start;
   tkr->p = te;
   return start;

   (void)stack;
   (void)sentencize_en_main;
   (void)sentencize_en_find_eos;
}
