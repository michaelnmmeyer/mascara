#include <string.h>
#include "lib/utf8proc.h"
#include "api.h"
#include "classify.h"

// FIXME find that automatically.
#define MAX_FEATURE_LEN 127 /* So far, longest = 65 on Tiger. */

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
   mr_bayes_load(&tkr->bayes, "models/fr_sequoia.mdl");
   mr_tokenizer_init(&tkr->tkr, vtab);
}

static void mr_classifier_fini(struct mascara *imp)
{
   struct mr_classifier *tkr = (struct mr_classifier *)imp;
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
