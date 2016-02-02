%%{

machine mr_sentencize2;

alphtype unsigned char;

include whitespace "whitespace.rl";
include misc "misc.rl";
include abbr_lexicon "abbr_lexicon.rl";
include symbol "symbol.rl";


#-------------------------------------------------------------------------------
# Character classes.
#-------------------------------------------------------------------------------

eos_marker = ellipsis | "?"+ | "!"+;

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
thing_with_periods = latin+ ("." "-"? latin+)+;

# Discard when followed by a lowercase letter, including exclamations:
#
#    Ah! princesse.
#    "Good gracious!" cried Mrs. Bennet.
#     "Why?" said Albert
#
no_capital = eos_marker sent_trail* (whitespace - paragraph_break)* sent_lead* latin_lowercase;

not_eos = thing_with_periods | no_capital | email | uri;

#-------------------------------------------------------------------------------
# Main.
#-------------------------------------------------------------------------------

find_eos := |*

   "." sent_trail* => {
      const struct mr_token *rhs = fetch_tokens(tkr, ts + 1);
      if (!rhs) {
         /* Sentence has grown too large. */
         goto fini;
      } else if (rhs[-1].len != 1 || *rhs[-1].str != '.') {
         /* Mismatch in tokenization (should not happen). FIXME log? */
         ;
      } else if (at_eos(tkr, rhs)) {
         fetch_tokens(tkr, te);
         goto fini;
      } else {
         reattach_period(tkr);
      }
   };

   eos_marker sent_trail* => {
      fetch_tokens(tkr, te);
      goto fini;
   };

   paragraph_break => {
      fetch_tokens(tkr, ts);
      goto fini;
   };

   any | not_eos;
*|;

main := whitespace* %{ start = fpc; }
        (code_point - whitespace) @{ fexec start; fcall find_eos; };

}%%

%% write data noerror nofinal;

static size_t mr_sentencize2_next(struct sentencizer2 *tkr, struct mr_token **tks)
{
   int cs, act, top, stack[1];
   const unsigned char *ts, *te;
   const unsigned char *p = tkr->p;
   const unsigned char *pe = tkr->pe;
   const unsigned char *const eof = pe;

   const unsigned char *start = NULL;

   %% write init;
   %% write exec;

   /* At EOS, flush the remaining tokens. */
   te = eof;
   fetch_tokens(tkr, te);

fini: {
   const size_t len = tkr->len - 2;
   tkr->p = te;
   *tks = len ? &tkr->tokens[1] : NULL;
   return len;

   (void)stack;
   (void)mr_sentencize2_en_main;
   (void)mr_sentencize2_en_find_eos;
}
}
