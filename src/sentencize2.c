#include <string.h>
#include "lib/utf8proc.h"
#include "api.h"
#include "sentencize2.h"
#include "mem.h"

static bool first_upper(const struct mr_token *tk)
{
   const ssize_t len = tk->len < 4 ? tk->len : 4;
   int32_t c;
   if (utf8proc_iterate((const uint8_t *)tk->str, len, &c) > 0)
      return utf8proc_get_property(c)->category == UTF8PROC_CATEGORY_LU;
   return false;
}

/*******************************************************************************
 * Features.
 ******************************************************************************/

/* A feature that won't match. */
static char *mr_ft_void(char *buf)
{
   *buf = 0xff;
   return buf + 1;
}

static char *strzcat(char *restrict buf, const char *restrict str)
{
   while (*str)
      *buf++ = *str++;
   return buf;
}

char *mr_ft_prefix4(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t pfx_len = 0;
   size_t nr = 4;
   while (pfx_len < len && nr) {
      const size_t clen = utf8proc_utf8class[str[pfx_len]];
      if (!clen || pfx_len + clen > len)
         break;
      pfx_len += clen;
      nr--;
   }
   memcpy(buf, str, pfx_len);
   return &buf[pfx_len];
}

char *mr_ft_suffix3(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t nr = 3;
   while (len)
      if ((str[--len] & 0xc0) != 0x80 && !--nr)
         break;

   const size_t sfx_len = tk->len - len;
   memcpy(buf, &str[len], sfx_len);
   return &buf[sfx_len];
}

char *mr_ft_len(char *buf, const struct mr_token *tk)
{
   static const char *const tbl[] = {
      [0] = "0",
      [1] = "1",
      [2] = "2..3",
      [3] = "2..3",
      [4] = "4..5",
      [5] = "4..5",
      [6] = "6..7",
      [7] = "6..7",
   };
   return strzcat(buf, tk->len < 8 ? tbl[tk->len] : "8..");
}

char *mr_ft_word(char *buf, const struct mr_token *tk)
{
   if (tk->len > MAX_FEATURE_LEN)
      return mr_ft_void(buf);
   memcpy(buf, tk->str, tk->len);
   return &buf[tk->len];
}

char *mr_ft_case(char *buf, const struct mr_token *tk)
{
   memcpy(buf, first_upper(tk) ? "LCAP" : "LLOW", 4);
   return &buf[4];
}

char *mr_ft_shape(char *buf, const struct mr_token *tk)
{
   if (tk->type == MR_LATIN)
      return mr_ft_case(buf, tk);
   return strzcat(buf, mr_token_type_name(tk->type));
}

#include "gen/vowel.ic"

char *mr_ft_mask(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;
   
   /* Technically, could match, but MAX_FEATURE_LEN is quite large. */
   if (len > MAX_FEATURE_LEN)
      return mr_ft_void(buf);

   ssize_t clen;
   for (size_t i = 0; i < len; i += clen) {
      int32_t c;
      clen = utf8proc_iterate(&str[i], len - i, &c);
      if (clen <= 0)
         break;
      switch (utf8proc_get_property(c)->category) {
      /* Letter. */
      case UTF8PROC_CATEGORY_LU:
      case UTF8PROC_CATEGORY_LL:
      case UTF8PROC_CATEGORY_LT:
      case UTF8PROC_CATEGORY_LM:
      case UTF8PROC_CATEGORY_LO:
         *buf++ = mr_is_vowel(c) ? 'V' : 'C';
         break;
      /* Digit. */
      case UTF8PROC_CATEGORY_ND:
      case UTF8PROC_CATEGORY_NL:
      case UTF8PROC_CATEGORY_NO:
         *buf++ = 'D';
         break;
      /* Other (punctuation, probably). */
      default:
         *buf++ = 'P';
         break;
      }
   }
   return buf;
}


/*******************************************************************************
 * Concrete implementations.
 ******************************************************************************/

#include "fr_sequoia.cm"
#include "en_amalg.cm"
#include "de_tiger.cm"


/*******************************************************************************
 * Interface.
 ******************************************************************************/

static void mr_sentencizer2_set_text(struct mascara *,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr);

static size_t mr_sentencizer2_next(struct mascara *, struct mr_token **);

static void mr_sentencizer2_fini(struct mascara *);

static const struct mr_imp mr_sentencizer2_imp = {
   .set_text = mr_sentencizer2_set_text,
   .next = mr_sentencizer2_next,
   .fini = mr_sentencizer2_fini,
};

MR_LOCAL int mr_sentencizer2_init(struct mr_sentencizer2 *tkr,
                                  const struct mr_tokenizer_vtab *vtab)
{
   (void)fr_sequoia_config;
   (void)de_tiger_config;
   (void)en_amalg_config;

   const struct mr_sentencizer2_config *cfg = &fr_sequoia_config;

   *tkr = (struct mr_sentencizer2){.base.imp = &mr_sentencizer2_imp};

   const char *home = mr_home;
   if (!home)
      return MR_EHOME;

   char path[4096];
   int len = snprintf(path, sizeof path, "%s/%s.mdl", home, cfg->bayes_config.name);
   if (len < 0 || (size_t)len >= sizeof path)
      return MR_EHOME;

   int ret = mr_bayes_load(&tkr->bayes, path, &cfg->bayes_config);
   if (ret)
      return ret;

   tkr->at_eos = cfg->at_eos;
   mr_tokenizer_init(&tkr->tkr, vtab);
   return MR_OK;
}

static void mr_sentencizer2_fini(struct mascara *imp)
{
   struct mr_sentencizer2 *tkr = (struct mr_sentencizer2 *)imp;
   mr_bayes_dealloc(tkr->bayes);
   free(tkr->tokens);
}

static void add_token(struct mr_sentencizer2 *tkr, const struct mr_token *tk)
{
   if (tkr->len == tkr->alloc) {
      tkr->alloc = tkr->alloc * 2 + 4;
      tkr->tokens = mr_realloc(tkr->tokens, tkr->alloc * sizeof *tkr->tokens);
   }
   tkr->tokens[tkr->len++] = *tk;
}

static void mr_sentencizer2_set_text(struct mascara *imp,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr)
{
   struct mr_sentencizer2 *tkr = (void *)imp;

   mr_tokenizer_set_text(&tkr->tkr.base, str, len, offset_incr);
   tkr->p = str;
   tkr->pe = &str[len];

   tkr->len = 0;
   add_token(tkr, &(struct mr_token){
      .str = (const char *)str,
      .offset = 0,
      .len = 0,
      .type = MR_UNK,
   });
   tkr->first = true;
}

static struct mr_token *fetch_tokens(struct mr_sentencizer2 *szr,
                                     const unsigned char *end)
{
   struct mr_token *tk = &szr->tokens[szr->len - 1];

   for (;;) {
      if (tk->str >= (const char *)end)
         return &szr->tokens[szr->len - 1];
      if (!mr_tokenizer_next(&szr->tkr.base, &tk)) {
         add_token(szr, &(struct mr_token){
            .str = (const char *)szr->pe,
            .len = 0,
            .offset = 0,
            .type = MR_UNK,
         });
         return &szr->tokens[szr->len - 1];
      }
      add_token(szr, tk);
   };
}

/* Conditions for reattaching a period are:
 * - There must be a single period (no ellipsis).
 * - This must not be the last period in the input sentence.
 * - The previous token is a likely abbreviation (type latin or abbr), not a
 *   symbol, etc.
 * - The period immediately follows the previous token.
 */
static void reattach_period(struct mr_sentencizer2 *szr)
{
   struct mr_token *period = &szr->tokens[szr->len - 2];
   
   assert(*period->str == '.' && period->len == 1);
   
   struct mr_token *prev = period - 1;
   if (prev->offset + prev->len != period->offset)
      return;
   
   switch (prev->type) {
   case MR_ABBR:
   case MR_LATIN:
      prev->len++;
      *period = period[1];
      szr->len--;
      break;
   default:
      break;
   }
}

#include "gen/sentencize2.ic"

static size_t mr_sentencizer2_next(struct mascara *imp, struct mr_token **tks)
{
   struct mr_sentencizer2 *szr = (void *)imp;
   assert(szr->p && "text no set");

   if (!szr->first) {
      szr->tokens[0] = szr->tokens[szr->len - 2];
      szr->tokens[1] = szr->tokens[szr->len - 1];
      szr->len = 2;
   } else {
      szr->len = 1;
      szr->first = false;
   }
   return mr_sentencize2_next(szr, tks);
}
