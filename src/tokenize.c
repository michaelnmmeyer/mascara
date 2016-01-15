#include "api.h"
#include "tokenize.h"
#include "gen/en_tokenize.ic"
#include "gen/fr_tokenize.ic"

const struct mr_tokenizer_vtab *mr_find_tokenizer(const char *name)
{
   static const struct mr_tokenizer_vtab tbl[] = {
   #define _(name) {#name, name##_init, name##_exec},
      _(en)
      _(fr)
   #undef _
      {NULL, 0, 0}
   };
   
   for (const struct mr_tokenizer_vtab *tk = tbl; tk->name; tk++)
      if (!strcmp(tk->name, name))
         return tk;
   return NULL;
}

const struct mr_imp mr_tokenizer_imp = {
   .set_text = mr_tokenizer_set_text,
   .next = mr_tokenizer_next,
};

void mr_tokenizer_init(struct mr_tokenizer *tkr,
                       const struct mr_tokenizer_vtab *vtab)
{
   *tkr = (struct mr_tokenizer){
      .base.imp = &mr_tokenizer_imp,
      .vtab = vtab,
   };
}

void mr_tokenizer_set_text(struct mascara *imp,
                           const unsigned char *s, size_t len,
                           size_t offset_incr)
{
   struct mr_tokenizer *tkr = (struct mr_tokenizer *)imp;

   tkr->str = tkr->p = s;
   tkr->pe = tkr->eof = &s[len];
   tkr->suffix_len = 0;
   tkr->offset_incr = offset_incr;
   tkr->vtab->init(tkr);
}

size_t mr_tokenizer_next(struct mascara *imp, struct mr_token **tkp)
{
   struct mr_tokenizer *tkr = (struct mr_tokenizer *)imp;
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
