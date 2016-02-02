#include <stdbool.h>
#include "api.h"
#include "sentencize.h"
#include "mem.h"

static void sentencizer_fini(struct mascara *imp)
{
   struct sentencizer *tkr = (struct sentencizer *)imp;
   free(tkr->tokens);
}

static void sentencizer_set_text(struct mascara *imp,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr)
{
   struct sentencizer *tkr = (struct sentencizer *)imp;

   tkr->offset_incr = offset_incr;
   tkr->str = tkr->p = str;
   tkr->pe = &str[len];
}

static void sentencizer_add_token(struct sentencizer *tkr, const struct mr_token *tk)
{
   if (tkr->len == tkr->alloc) {
      tkr->alloc = tkr->alloc * 2 + 4;
      tkr->tokens = mr_realloc(tkr->tokens, tkr->alloc * sizeof *tkr->tokens);
   }
   tkr->tokens[tkr->len++] = *tk;
}

static bool sentencizer_reattach_period(struct sentencizer *szr, const struct mr_token *tk)
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

#include "gen/sentencize.ic"

static size_t sentencizer_next(struct mascara *imp, struct mr_token **tks)
{
   struct sentencizer *szr = (struct sentencizer *)imp;
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

   struct tokenizer tkr;
   tokenizer_init(&tkr, szr->vtab);
   tokenizer_set_text(&tkr.base, str, len, offset_incr);

   struct mr_token *tk;
   while (tokenizer_next(&tkr.base, &tk)) {
      if (tk->str == (const char *)last_period || !sentencizer_reattach_period(szr, tk)) {
         sentencizer_add_token(szr, tk);
         if (szr->len == MR_MAX_SENTENCE_LEN) {
            szr->p = (const unsigned char *)tk->str + tk->len;
            break;
         }
      }
   }
   *tks = szr->tokens;
   return szr->len;
}

static const struct mr_imp sentencizer_imp = {
   .set_text = sentencizer_set_text,
   .next = sentencizer_next,
   .fini = sentencizer_fini,
};

local void sentencizer_init(struct sentencizer *tkr,
                            const struct tokenizer_vtab *vtab)
{
   *tkr = (struct sentencizer){
      .base.imp = &sentencizer_imp,
      .vtab = vtab,
   };
}
