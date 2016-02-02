#include "api.h"
#include "tokenize.h"

const struct mr_imp mr_tokenizer_imp = {
   .set_text = tokenizer_set_text,
   .next = tokenizer_next,
};

local void tokenizer_init(struct tokenizer *tkr,
                          const struct tokenizer_vtab *vtab)
{
   *tkr = (struct tokenizer){
      .base.imp = &mr_tokenizer_imp,
      .vtab = vtab,
   };
}

local void tokenizer_set_text(struct mascara *imp,
                              const unsigned char *s, size_t len,
                              size_t offset_incr)
{
   struct tokenizer *tkr = (void *)imp;

   tkr->str = tkr->p = s;
   tkr->pe = tkr->eof = &s[len];
   tkr->suffix_len = 0;
   tkr->offset_incr = offset_incr;
   tkr->vtab->init(tkr);
}

local size_t tokenizer_next(struct mascara *imp, struct mr_token **tkp)
{
   struct tokenizer *tkr = (void *)imp;
   assert(tkr->str && "text no set");

   struct mr_token *tk = &tkr->token;

   if (tkr->suffix_len) {
      tk->type = MR_SUFFIX;
      tk->str = (const char *)(tkr->te - tkr->suffix_len);
      tk->len = tkr->suffix_len;
      tk->offset = tk->str - (const char *)tkr->str + tkr->offset_incr;
      tkr->suffix_len = 0;
      *tkp = tk;
      return 1;
   }

   tk->str = NULL;
   tkr->vtab->exec(tkr, tk);
   if (tk->str) {
      *tkp = tk;
      return 1;
   }
   *tkp = NULL;
   return 0;
}
