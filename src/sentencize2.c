#include <string.h>
#include "api.h"
#include "sentencize2.h"
#include "mem.h"
#include "features.h"

#include "de_tiger.cm"
#include "en_amalg.cm"
#include "fr_sequoia.cm"
#include "it_tut.cm"

local const struct sentencizer2_config *find_sentencizer2(const char *lang)
{
   static const struct {
      const char *lang;
      const struct sentencizer2_config *cfg;
   } tbl[] = {
      {"de", &de_tiger_config},
      {"en", &en_amalg_config},
      {"fr", &fr_sequoia_config},
      {"it", &it_tut_config},
   };
   
   for (size_t i = 0; i < sizeof tbl / sizeof *tbl; i++)
      if (!strcmp(tbl[i].lang, lang))
         return tbl[i].cfg;
   return NULL;
}

local void sentencizer2_fini(struct mascara *imp)
{
   struct sentencizer2 *tkr = (void *)imp;
   bayes_dealloc(tkr->bayes);
   free(tkr->sent.tokens);
   kb_fini(&tkr->lhs);
   kb_fini(&tkr->rhs);
}

local void sentencizer2_set_text(struct mascara *imp,
                                 const unsigned char *str, size_t len,
                                 size_t offset_incr)
{
   struct sentencizer2 *tkr = (void *)imp;

   tokenizer_set_text(&tkr->tkr.base, str, len, offset_incr);
   tkr->p = str;
   tkr->pe = &str[len];
   sentence_clear(&tkr->sent);
}

local struct mr_token *fetch_tokens(struct sentencizer2 *szr,
                                    const unsigned char *end)
{
   struct sentence *sent = &szr->sent;
   struct mr_token *tk = &sent->tokens[sent->len - 1];

   while (sent->len < MR_MAX_SENTENCE_LEN + 2) {
      if (tk->str >= (const char *)end)
         return &sent->tokens[sent->len - 1];
      if (!tokenizer_next(&szr->tkr.base, &tk)) {
         sentence_add(sent, &(struct mr_token){
            .str = (const char *)szr->pe,
            .type = MR_UNK,
         });
         break;
      }
      sentence_add(sent, tk);
   };
   return NULL;
}

local void sentencizer2_reattach_period(struct sentence *sent)
{
   struct mr_token *period = &sent->tokens[sent->len - 2];

   if (can_reattach_period(&period[-1], period)) {
      period[-1].len += period->len;
      *period = period[1];
      sent->len--;
   }
}

local bool at_eos(struct sentencizer2 *szr, const struct mr_token *rhs)
{
   const struct mr_token *lhs = &rhs[-2];
   const unsigned opts = KB_NFKC | KB_STRIP_IGNORABLE | KB_STRIP_UNKNOWN;

   kb_transform(&szr->lhs, lhs->str, lhs->len, opts);
   if (szr->lhs.len > MAX_FEATURE_LEN)
      goto fail;

   kb_transform(&szr->rhs, rhs->str, rhs->len, opts);
   if (szr->rhs.len > MAX_FEATURE_LEN)
      goto fail;

   const struct mr_token ltk = {
      .str = szr->lhs.str,
      .len = szr->lhs.len,
      .type = lhs->type,
   };
   const struct mr_token rtk = {
      .str = szr->rhs.str,
      .len = szr->rhs.len,
      .type = rhs->type,
   };
   return szr->at_eos(szr->bayes, &ltk, &rtk);

fail:
   /* We cannot proceed, so choose the most likely solution. */
   return true;
}

#include "gen/sentencize2.ic"

local size_t sentencizer2_next(struct mascara *imp, struct mr_token **tks)
{
   struct sentencizer2 *szr = (void *)imp;
   struct sentence *sent = &szr->sent;
   assert(szr->p && "text no set");

   /* Pending tokens? */
   if (sent->len) {
      sent->tokens[0] = sent->tokens[sent->len - 2];
      sent->tokens[1] = sent->tokens[sent->len - 1];
      sent->len = 2;
   } else {
      sentence_add(sent, &(struct mr_token){
         .str = (const char *)szr->p,
         .type = MR_UNK,
      });
   }
   return mr_sentencize2_next(szr, tks);
}

local const struct mr_imp sentencizer2_imp = {
   .set_text = sentencizer2_set_text,
   .next = sentencizer2_next,
   .fini = sentencizer2_fini,
};

local int sentencizer2_init(struct sentencizer2 *tkr,
                            const struct tokenizer_vtab *vtab,
                            const struct sentencizer2_config *cfg)
{
   *tkr = (struct sentencizer2){
      .base.imp = &sentencizer2_imp,
      .lhs = KB_INIT,
      .rhs = KB_INIT,
   };

   const char *home = mr_home;
   if (!home)
      return MR_EHOME;

   char path[4096];
   int len = snprintf(path, sizeof path, "%s/%s.mdl", home, cfg->bayes_config.name);
   if (len < 0 || (size_t)len >= sizeof path)
      return MR_EHOME;

   int ret = bayes_load(&tkr->bayes, path, &cfg->bayes_config);
   if (ret)
      return ret;

   tkr->at_eos = cfg->at_eos;
   tokenizer_init(&tkr->tkr, vtab);
   return MR_OK;
}
