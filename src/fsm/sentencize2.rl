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
thing_with_periods = latin+ (period hyphen? latin+)+;

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

   period sent_trail* => {
      const struct mr_token *rhs = fetch_tokens(tkr, ts + 1);
      
      /* If the sentence has grown too large, stop there. */
      if (!rhs) {
         goto fini;
      /* If there is tokenization mismatch between ourselves and the tokenizer,
       * (should not happen!), don't do anything.
       */
      } else if (rhs[-1].str != (const char *)ts) {
         ;
      /* If we're at eos, fetch the tokens trailing the current sentence and
       * stop there.
       */
      } else if (at_eos(tkr, rhs)) {
         fetch_tokens(tkr, te);
         goto fini;
      /* Otherwise, attempt to reattach the period to the token that precedes
       * it.
       */
      } else {
         sentencizer2_reattach_period(&tkr->sent);
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
   if (tkr->sent.len < 2)
      tkr->sent.len = 2;

fini: {
   const size_t len = tkr->sent.len - 2;
   tkr->p = te;
   *tks = len ? &tkr->sent.tokens[1] : NULL;
   return len;

   (void)stack;
   (void)mr_sentencize2_en_main;
   (void)mr_sentencize2_en_find_eos;
}
}
