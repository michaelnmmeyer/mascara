/* Generated file, don't edit! */
#include "en_suffix_match.ic"

%%{

machine tokenize_en;

alphtype unsigned char;

access tkr->;
variable p tkr->p;
variable pe tkr->pe;
variable eof tkr->eof;

include symbol "symbol.rl";
include whitespace "whitespace.rl";
include numeric "numeric.rl";
include latin "latin.rl";
include misc "misc.rl";
include word "en_word.rl";
include en_lexicon "en_lexicon.rl";
include en_assimilation "en_assimilation.rl";

# Patterns order is significant: when there is an ambiguity, the first ones
# take precedence.

main := |*
   symbol         => { SAVE(MR_SYM); fbreak; };
   path           => { SAVE(MR_PATH); fbreak; };
   uri            => { SAVE(MR_URI); fbreak; };
   abbreviation   => { SAVE(MR_ABBR); fbreak; };
   numeric        => { SAVE(MR_NUM); fbreak; };
   en_lexicon  => { SAVE(MR_LATIN); fbreak; };

   # A one-code-point lookahead is needed for elisions, to avoid tokenizing
   # strings like:
   #    la lettre 'c'
   # as:
   #    la, lettre, ', c'
   # where we should have:
   #    la, lettre, ', c, '
   # Sometimes people insert a whitespace after an elision:
   #    c' est vrai
   # We choose not to deal with that.
   #
   # Examples:
   #    c'est
   #    l'"ouverture"
   elision (latin_letter | double_quote) => {
      /* Drop one code point. */
      do tkr->te--;
      while ((*tkr->te & 0xc0) == 0x80);
      fexec tkr->te;
      SAVE(MR_PREFIX);
      fbreak;
   };

   # After matching a word, we need to check if a suffix follows. This only
   # applies to the word pattern (_not_ the lexicon pattern, etc.), so we do
   # that directly in the machine.
   word => {
      SAVE(MR_LATIN);
      tkr->suffix_len = en_suffix(tkr->te - 1, tkr->ts);
      tk->len -= tkr->suffix_len;
      fbreak;
   };
   
   email           => { SAVE(MR_EMAIL); fbreak; };
   unknown         => { SAVE(MR_UNK); fbreak; };
   whitespace+;
   
   # Split assimilations: 'twas -> 't, was. Only done for English.
   en_assimilation => {
      tkr->te = tkr->ts + 2;
      while (tkr->te[-1] != 't' && tkr->te[-1] != 'T')
         tkr->te++;
      fexec tkr->te;
      SAVE(MR_LATIN);
      fbreak;
   };
*|;

}%%

#define SAVE(t) do {                                                           \
   tk->type = (t);                                                             \
   tk->str = (const char *)tkr->ts;                                            \
   tk->offset = tkr->ts - tkr->str + tkr->offset_incr;                         \
   tk->len = tkr->te - tkr->ts;                                                \
} while (0)

%% write data noerror nofinal;

static void en_init(struct mr_tokenizer *tkr)
{
   %% write init;
}

static void en_exec(struct mr_tokenizer *tkr, struct mr_token *tk)
{
   %% write exec;
   (void)tokenize_en_en_main;
}
