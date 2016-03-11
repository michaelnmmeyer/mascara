#include <stdlib.h>
#include <stdio.h>
#include <string.h>

%%{

machine grapheme_split;

alphtype unsigned char;

access tkr->;
variable p tkr->p;
variable pe tkr->pe;
variable eof tkr->eof;

include grapheme "grapheme.rl";

main := |*
   grapheme => {
      tk->type = MR_UNK;
      tk->str = (const char *)tkr->ts;
      tk->offset = tkr->ts - tkr->str + tkr->offset_incr;
      tk->len = tkr->te - tkr->ts;
      fbreak;
   };
*|;

}%%

%% write data noerror nofinal;

static void grapheme_split_init(struct graphemizer *tkr)
{
   %% write init;
}

static void grapheme_split_exec(struct graphemizer *tkr, struct mr_token *tk)
{
   %% write exec;
}
