%%{

machine sentencize;

alphtype unsigned char;

include whitespace "whitespace.rl";
include misc "misc.rl";
include abbr_lexicon "abbr_lexicon.rl";


#-------------------------------------------------------------------------------
# Character classes.
#-------------------------------------------------------------------------------

eos_marker = "." | "?" | "!";

closing_bracket = ")" | "]" | "}";

closing_quote = closing_single_quote | closing_double_quote;


#-------------------------------------------------------------------------------
# Where to split.
#-------------------------------------------------------------------------------

# Symbols that can appear at the end of a sentence, after the terminal period.
# Only useful for English. In French, these should appear _before_ the
# terminal period.
eos_trail = closing_bracket? closing_single_quote? closing_double_quote?
          | closing_single_quote? closing_double_quote? closing_bracket?
          ;

eos = eos_marker+ eos_trail;


#-------------------------------------------------------------------------------
# Where _not_ to split.
#-------------------------------------------------------------------------------

# Alphanumeric containing internal periods: Ph.D., 1.2.c, e.g., i.e.
thing_with_periods = latin+ ("." latin+)+ "."?;

# A token followed by a period followed by a comma is an abbreviation.
period_comma = latin+ "." eos_trail ",";

# John J. Doe.
name_initial = latin_uppercase "." whitespace latin_uppercase;

# Example:
#    See p. 5 for more informations.
page_number = "p." whitespace digit;

consonant = [BCDFGHJKLMNPQRSTVWXZ] | [bcdfghjklmnpqrstvwxz];

# Example:
#    cf.
abbreviation_consonant = consonant+ ".";

# Interjection:
#    Ah! princesse.
#    "Good gracious!" cried Mrs. Bennet.
# Note that (latin - latin_uppercase) doesn't necessarily match lowercase
# letters.
interjection = latin+ "!" closing_quote? whitespace (latin - latin_uppercase);

not_eos = thing_with_periods
        | period_comma
        | name_initial
        | page_number
        | interjection
        | abbr_lexicon
        | abbreviation_consonant
        | email | uri
        ;

#-------------------------------------------------------------------------------
# Main.
#-------------------------------------------------------------------------------

find_eos := |*
   "." eos_trail => {
      *period = ts;
      goto found;
   };
   eos => {
      goto found;
   };
   paragraph_break => {
      /* Don't include the paragraph break itself in the sentence. */
      te = ts;
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

static const unsigned char *next_sentence(struct mr_sentencizer *tkr,
                                          size_t *len,
                                          const unsigned char **period)
{
   int cs, act, top, stack[1];
   const unsigned char *ts, *te;
   const unsigned char *p = tkr->p;
   const unsigned char *pe = tkr->pe;
   const unsigned char *const eof = pe;

   const unsigned char *start = NULL;
   *period = NULL;

   %% write init;
   %% write exec;

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
