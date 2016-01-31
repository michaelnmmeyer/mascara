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
   };
   
   const struct mr_classifier_config *cfg = &mr_fr_sequoia_config;
   tkr->cfg = cfg;
   mr_bayes_load(&tkr->bayes, "models/fr_sequoia.mdl", &cfg->bayes_config);

   mr_tokenizer_init(&tkr->tkr, vtab);
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
   mr_tokenizer_set_text(&tkr->tkr.base, str, len, offset_incr);
}

static size_t mr_classifier_next(struct mascara *imp, struct mr_token **tks)
{
   struct mr_classifier *szr = (struct mr_classifier *)imp;

   (void)szr; (void)imp; (void)tks;
   return 0;
}
