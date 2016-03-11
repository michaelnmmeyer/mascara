#include "graphemize.h"
#include "api.h"
#include "imp.h"

#include "gen/grapheme_split.ic"

local void graphemizer_set_text(struct mascara *imp,
                                const unsigned char *s, size_t len,
                                size_t offset_incr)
{
   struct graphemizer *tkr = (void *)imp;

   tkr->str = tkr->p = s;
   tkr->pe = tkr->eof = &s[len];
   tkr->offset_incr = offset_incr;
   grapheme_split_init(tkr);
}

local size_t graphemizer_next(struct mascara *imp, struct mr_token **tkp)
{
   struct graphemizer *tkr = (void *)imp;
   assert(tkr->str && "text no set");

   struct mr_token *tk = &tkr->token;

   tk->str = NULL;
   grapheme_split_exec(tkr, tk);
   if (tk->str) {
      *tkp = tk;
      return 1;
   }
   *tkp = NULL;
   return 0;
}

const struct mr_imp mr_graphemizer_imp = {
   .set_text = graphemizer_set_text,
   .next = graphemizer_next,
};

local void graphemizer_init(struct graphemizer *tkr)
{
   *tkr = (struct graphemizer){
      .base.imp = &mr_graphemizer_imp,
   };
}
