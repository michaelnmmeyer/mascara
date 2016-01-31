#include <string.h>
#include "lib/utf8proc.h"
#include "api.h"
#include "classify.h"

/*******************************************************************************
 * Features.
 ******************************************************************************/

#define MAX_FEATURE_LEN 255

static char *strzcat(char *restrict buf, const char *restrict str)
{
   while (*str)
      *buf++ = *str++;
   return buf;
}

char *ft_suffix(char buf[static MAX_FEATURE_LEN + 1], const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t nr = 4;
   while (len)
      if ((str[--len] & 0xc0) != 0x80 && !--nr)
         break;

   memcpy(buf, &str[len], tk->len - len);
   return &buf[tk->len - len];
}

char *ft_len(char buf[static MAX_FEATURE_LEN + 1], const struct mr_token *tk)
{
   static const char *const tbl[] = {
      [0] = "0",
      [1] = "1",
      [2] = "2..3",
      [3] = "2..3",
      [4] = "4..5",
      [5] = "4..5",
   };
   return strzcat(buf, tk->len < 6 ? tbl[tk->len] : "6..");
}

char *ft_word(char buf[static MAX_FEATURE_LEN + 1], const struct mr_token *tk)
{
   if (tk->len > MAX_FEATURE_LEN) {
      buf[0] = '\xff';  /* Won't find it. */
      return buf + 1;
   }
   memcpy(buf, tk->str, tk->len);
   return &buf[tk->len];
}

static bool first_upper(const struct mr_token *tk)
{
   const ssize_t len = tk->len < 4 ? tk->len : 4;
   int32_t c;
   if (utf8proc_iterate((const uint8_t *)tk->str, len, &c) > 0)
      return utf8proc_get_property(c)->category == UTF8PROC_CATEGORY_LU;
   return false;
}

char *ft_shape(char buf[static MAX_FEATURE_LEN + 1], const struct mr_token *tk)
{
   const char *t;
   if (tk->type == MR_LATIN)
      t = first_upper(tk) ? "LCAP" : "LLOW";
   else
      t = mr_token_type_name(tk->type);
   return strzcat(buf, t);
}

char *ft_case(char buf[static MAX_FEATURE_LEN + 1], const struct mr_token *tk)
{
   memcpy(buf, first_upper(tk) ? "LCAP" : "LLOW", 4);
   return buf + 4;
}


/*******************************************************************************
 * Concrete implementation.
 ******************************************************************************/

bool fr_sequoia_eos(const struct mr_bayes *mdl, const struct mr_token *left, const struct mr_token *right)
{
   double vec[2];
   char stack[MAX_FEATURE_LEN + 1], *buf;

   mr_bayes_init(mdl, vec);

   buf = stack;
   *buf++ = 1;
   buf = ft_shape(buf, left);
   *buf++ = '+';
   buf = ft_len(buf, left);
   *buf = '\0';
   mr_bayes_feed(mdl, vec, stack);

   buf = stack;
   *buf++ = 2;
   buf = ft_word(buf, left);
   *buf++ = '+';
   buf = ft_shape(buf, right);
   *buf = '\0';
   mr_bayes_feed(mdl, vec, stack);

   return vec[MR_EOS] >= vec[MR_NOT_EOS];
}

struct mr_classifier_config {
   const char *name;
   const struct mr_bayes_config bayes_config;
   bool (*eos)(const struct mr_bayes *mdl, const struct mr_token *, const struct mr_token *);
};

static const char *const fr_sequoia_features[] = {
   "l_shape+l_len",
   "l_word+r_shape",
   NULL
};

static const struct mr_classifier_config mr_fr_sequoia_config = {
   .name = "fr_sequoia",
   .bayes_config = {
      .signature = "fr_sequoia",
      .features = fr_sequoia_features,
   },
   .eos = fr_sequoia_eos,
};


/*******************************************************************************
 * Interface.
 ******************************************************************************/

static void mr_classifier_set_text(struct mascara *,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr);

static size_t mr_classifier_next(struct mascara *, struct mr_token **);

static void mr_classifier_fini(struct mascara *);

static const struct mr_imp mr_classifier_imp = {
   .set_text = mr_classifier_set_text,
   .next = mr_classifier_next,
   .fini = mr_classifier_fini,
};

void mr_classifier_init(struct mr_classifier *tkr,
                        const struct mr_tokenizer_vtab *vtab)
{
   *tkr = (struct mr_classifier){
      .base.imp = &mr_classifier_imp,
      .vtab = vtab,
   };
   
   const struct mr_classifier_config *cfg = &mr_fr_sequoia_config;
   tkr->cfg = cfg;
   mr_bayes_load(&tkr->bayes, "models/fr_sequoia.mdl", &cfg->bayes_config);
}

static void mr_classifier_fini(struct mascara *imp)
{
   struct mr_classifier *tkr = (struct mr_classifier *)imp;
   mr_bayes_dealloc(tkr->bayes);
   free(tkr->tokens);
}

static void mr_classifier_set_text(struct mascara *imp,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr)
{
   struct mr_classifier *tkr = (struct mr_classifier *)imp;
   tkr->offset_incr = offset_incr;
   tkr->p = str;
   tkr->pe = &str[len];
}

bool add_token(struct mr_classifier *tkr, const struct mr_token *tk)
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

bool reattach_period(struct mr_classifier *szr, const struct mr_token *tk)
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

#include "gen/sentencize2.ic"

static size_t mr_classifier_next(struct mascara *imp, struct mr_token **tks)
{
   struct mr_classifier *szr = (void *)imp;
   assert(szr->str && "text no set");

   szr->len = 0;

   size_t len;
   const unsigned char *last_period;
   const unsigned char *str = sentencize2_next(szr, &len, &last_period);
   if (!str) {
      *tks = NULL;
      return 0;
   }
   size_t offset_incr = szr->offset_incr + str - szr->str;

   struct mr_tokenizer tkr;
   mr_tokenizer_init(&tkr, szr->vtab);
   if (last_period)
      mr_tokenizer_set_text(&tkr.base, str, szr->pe - szr->p, offset_incr);
   else
      mr_tokenizer_set_text(&tkr.base, str, len, offset_incr);

   struct mr_token *tk;
   while (mr_tokenizer_next(&tkr.base, &tk)) {
      if (tk->str == (const char *)last_period) {
         if (szr->len == 0) {
            add_token(szr, tk);
            break;
         }
         struct mr_token left = *tk;
         if (!mr_tokenizer_next(&tkr.base, &tk)) {
            add_token(szr, tk);
            break;
         }
         bool eos = szr->cfg->eos(szr->bayes, &left, tk);
         if (eos) {
            add_token(szr, tk);
            break;
         }
      }
      if (!reattach_period(szr, tk)) {
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
