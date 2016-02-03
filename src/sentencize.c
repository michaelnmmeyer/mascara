#include <stdbool.h>
#include "api.h"
#include "sentencize.h"
#include "mem.h"

local void sentencizer_fini(struct mascara *imp)
{
   struct sentencizer *tkr = (struct sentencizer *)imp;
   free(tkr->sent.tokens);
}

local void sentencizer_set_text(struct mascara *imp,
                                const unsigned char *str, size_t len,
                                size_t offset_incr)
{
   struct sentencizer *tkr = (struct sentencizer *)imp;

   tkr->offset_incr = offset_incr;
   tkr->str = tkr->p = str;
   tkr->pe = &str[len];
}

local void sentence_add(struct sentence *sent, const struct mr_token *tk)
{
   if (sent->len == sent->alloc) {
      sent->alloc = sent->alloc * 2 + 4;
      sent->tokens = mr_realloc(sent->tokens, sent->alloc * sizeof *sent->tokens);
   }
   sent->tokens[sent->len++] = *tk;
}

local void sentence_clear(struct sentence *sent)
{
   sent->len = 0;
}

/* Conditions for reattaching a period to the token that precedes it are:
 * - There must be a single period (no ellipsis).
 * - This must not be the last period in the input text.
 * - The previous token must be a likely abbreviation (type latin or abbr), not
 *   a symbol, etc.
 * - The period immediately follows the previous token.
 */
local bool can_reattach_period(const struct mr_token *lhs,
                               const struct mr_token *period)
{
   assert(period->len == 1 && *period->str == '.');
   
   if (lhs->offset + lhs->len != period->offset)
      return false;
   
   switch (lhs->type) {
   case MR_ABBR:
   case MR_LATIN:
      return true;
   default:
      return false;
   }
}

local bool sentencizer_reattach_period(struct sentence *sent,
                                       const struct mr_token *tk)
{
   if (tk->len == 1 && *tk->str == '.' && sent->len) {
      struct mr_token *lhs = &sent->tokens[sent->len - 1];
      if (can_reattach_period(lhs, tk)) {
         lhs->len++;
         return true;
      }
   }
   return false;
}

#include "gen/sentencize.ic"

local size_t sentencizer_next(struct mascara *imp, struct mr_token **tks)
{
   struct sentencizer *szr = (struct sentencizer *)imp;
   struct sentence *sent = &szr->sent;

   assert(szr->str && "text no set");
   sentence_clear(sent);

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
      if (tk->str == (const char *)last_period ||
         !sentencizer_reattach_period(sent, tk)) {
         sentence_add(sent, tk);
         if (sent->len == MR_MAX_SENTENCE_LEN) {
            szr->p = (const unsigned char *)tk->str + tk->len;
            break;
         }
      }
   }
   *tks = sent->tokens;
   return sent->len;
}

local const struct mr_imp sentencizer_imp = {
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
