#include <stdbool.h>
#include "api.h"
#include "sentencize.h"
#include "gen/sentencize.ic"

static void mr_sentencizer_set_text(struct mascara *,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr);

static size_t mr_sentencizer_next(struct mascara *, struct mr_token **);

static void mr_sentencizer_fini(struct mascara *);

static const struct mr_imp mr_sentencizer_imp = {
   .set_text = mr_sentencizer_set_text,
   .next = mr_sentencizer_next,
   .fini = mr_sentencizer_fini,
};

void mr_sentencizer_init(struct mr_sentencizer *tkr,
                         const struct mr_tokenizer_vtab *vtab)
{
   *tkr = (struct mr_sentencizer){
      .base.imp = &mr_sentencizer_imp,
      .vtab = vtab,
   };
}

static void mr_sentencizer_fini(struct mascara *imp)
{
   struct mr_sentencizer *tkr = (struct mr_sentencizer *)imp;
   free(tkr->tokens);
}

static void mr_sentencizer_set_text(struct mascara *imp,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr)
{
   struct mr_sentencizer *tkr = (struct mr_sentencizer *)imp;

   tkr->offset_incr = offset_incr;
   tkr->str = tkr->p = str;
   tkr->pe = &str[len];
}

static bool add_token(struct mr_sentencizer *tkr, const struct mr_token *tk)
{
   if (tkr->len == tkr->alloc) {
      size_t new_alloc = tkr->alloc * 2 + 4;
      void *tokens = realloc(tkr->tokens, new_alloc * sizeof *tkr->tokens);
      if (!tokens) {
         tkr->base.err = MR_ENOMEM;
         return false;
      }
      tkr->alloc = new_alloc;
      tkr->tokens = tokens;
   }
   tkr->tokens[tkr->len++] = *tk;
   return true;
}

static bool reattach_period(struct mr_sentencizer *szr, const struct mr_token *tk)
{
   /* Conditions for reattaching a period are:
    * - There must be a single period (no ellipsis).
    * - This must not be the last period in the input text.
    * - The previous token is a likely abbreviation (type latin or abbr), not a
    *   symbol, etc.
    * - The period immediately follows the previous token.
    */
   if (*tk->str == '.' && tk->len == 1 && szr->len) {
      struct mr_token *prev = &szr->tokens[szr->len - 1];
      if (prev->offset + prev->len == tk->offset &&
          (prev->type == MR_ABBR || prev->type == MR_LATIN)) {
            prev->len++;
            return true;
      }
   }
   return false;
}

static size_t mr_sentencizer_next(struct mascara *imp, struct mr_token **tks)
{
   struct mr_sentencizer *szr = (struct mr_sentencizer *)imp;
   assert(szr->str && "text no set");

   szr->len = 0;
   
   size_t len;
   const unsigned char *last_period;
   const unsigned char *str = next_sentence(szr, &len, &last_period);
   if (!str) {
      *tks = NULL;
      return 0;
   }
   size_t offset_incr = szr->offset_incr + str - szr->str;
   
   struct mr_tokenizer tkr;
   mr_tokenizer_init(&tkr, szr->vtab);
   mr_tokenizer_set_text(&tkr.base, str, len, offset_incr);

   struct mr_token *tk;
   while (mr_tokenizer_next(&tkr.base, &tk)) {
      if (tk->str == (const char *)last_period || !reattach_period(szr, tk)) {
         if (!add_token(szr, tk))
            return 0;
         if (szr->len == MR_MAX_SENTENCE_LEN) {
            szr->p = (const unsigned char *)tk->str + tk->len;
            break;
         }
      }
   }
   *tks = szr->tokens;
   return szr->len;
}
