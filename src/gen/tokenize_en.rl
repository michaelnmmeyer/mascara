/* Generated file, don't edit! */
#include "suffix_en.ic"

%%{

machine tokenize_en;

alphtype unsigned char;

access mr->;
variable p mr->p;
variable pe mr->pe;
variable eof mr->eof;

include symbol "symbol.rl";
include whitespace "whitespace.rl";
include numeric "numeric.rl";
include latin "latin.rl";
include misc "misc.rl";
include word "word_en.rl";
include en_lexicon "en_lexicon.rl";
include en_assimilation "en_assimilation.rl";

# Patterns order is significant: when there is an ambiguity, the first ones
# take precedence.

main := |*
   symbol         => { SAVE(MR_SYM); fbreak; };
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
      do mr->te--;
      while ((*mr->te & 0xc0) == 0x80);
      fexec mr->te;
      SAVE(MR_ELISION);
      fbreak;
   };

   # After matching a word, we need to check if a suffix follows. This only
   # applies to the word pattern (_not_ the lexicon pattern, etc.), so we do
   # that directly in the machine.
   word => {
      SAVE(MR_LATIN);
      mr->suffix_len = en_suffix(mr->te - 1, mr->ts);
      tk->len -= mr->suffix_len;
      fbreak;
   };
   
   email           => { SAVE(MR_EMAIL); fbreak; };
   unknown         => { SAVE(MR_UNK); fbreak; };
   paragraph_break => {
      if (emit_para) {
         SAVE(MR_PARA_BREAK);
         fbreak;
      }
   };
   whitespace+;
   
   # Split assimilations: 'twas -> 't, was. Only done for English.
   en_assimilation => {
      mr->te = mr->ts + 2;
      while (mr->te[-1] != 't' && mr->te[-1] != 'T')
         mr->te++;
      fexec mr->te;
      SAVE(MR_LATIN);
      fbreak;
   };
*|;

}%%

#define SAVE(t) do {                                                           \
   tk->type = (t);                                                             \
   tk->str = (const char *)mr->ts;                                             \
   tk->offset = mr->ts - mr->str;                                              \
   tk->len = mr->te - mr->ts;                                                  \
} while (0)

%% write data;

static void en_init(struct mascara *mr)
{
   %% write init;
}

static void en_exec(struct mascara *mr, struct mr_token *tk, int emit_para)
{
	%% write exec;
}
