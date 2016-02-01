#include <string.h>
#include "lib/utf8proc.h"
#include "api.h"
#include "classify.h"
#include "mem.h"

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

char *ft_suffix(char buf[static MAX_FEATURE_LEN], const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t nr = 4;
   while (len)
      if ((str[--len] & 0xc0) != 0x80 && !--nr)
         break;

   const size_t sfx_len = tk->len - len;
   memcpy(buf, &str[len], sfx_len);
   return &buf[sfx_len];
}

char *ft_len(char buf[static MAX_FEATURE_LEN], const struct mr_token *tk)
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

char *ft_word(char buf[static MAX_FEATURE_LEN], const struct mr_token *tk)
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

char *ft_case(char buf[static MAX_FEATURE_LEN], const struct mr_token *tk)
{
   memcpy(buf, first_upper(tk) ? "LCAP" : "LLOW", 4);
   return buf + 4;
}

char *ft_shape(char buf[static MAX_FEATURE_LEN], const struct mr_token *tk)
{
   if (tk->type == MR_LATIN)
      return ft_case(buf, tk);
   return strzcat(buf, mr_token_type_name(tk->type));
}


/*******************************************************************************
 * Concrete implementation.
 ******************************************************************************/

bool fr_sequoia_at_eos(const struct mr_bayes *mdl,
                       const struct mr_token *left, const struct mr_token *right)
{
   double vec[2];
   char stack[MAX_FEATURE_LEN * 2 + 1], *buf;

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
   at_eos_fn *at_eos;
};

static const char *const fr_sequoia_features[] = {
   "l_shape+l_len",
   "l_word+r_shape",
   NULL
};

static const struct mr_classifier_config mr_fr_sequoia_config = {
   .name = "fr_sequoia",
   .bayes_config = {
      .signature = "fr_sequoia 1",
      .features = fr_sequoia_features,
   },
   .at_eos = fr_sequoia_at_eos,
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
   const struct mr_classifier_config *cfg = &mr_fr_sequoia_config;

   *tkr = (struct mr_classifier){.base.imp = &mr_classifier_imp};
   mr_tokenizer_init(&tkr->tkr, vtab);
   
   char path[4096];
   int len = snprintf(path, sizeof path, "models/%s.mdl", cfg->name);
   if (len < 0 || (size_t)len >= sizeof path) {
      fprintf(stderr, "path too long");
      abort();
   }
   int ret = mr_bayes_load(&tkr->bayes, path, &cfg->bayes_config);
   if (ret) {
      fprintf(stderr, "cannot load model: %s\n", mr_strerror(ret));
      abort();
   }
   tkr->at_eos = cfg->at_eos;
}

static void mr_classifier_fini(struct mascara *imp)
{
   struct mr_classifier *tkr = (struct mr_classifier *)imp;
   mr_bayes_dealloc(tkr->bayes);
   free(tkr->tokens);
}

void add_token(struct mr_classifier *tkr, const struct mr_token *tk)
{
   if (tkr->len == tkr->alloc) {
      tkr->alloc = tkr->alloc * 2 + 4;
      tkr->tokens = mr_realloc(tkr->tokens, tkr->alloc * sizeof *tkr->tokens);
   }
   tkr->tokens[tkr->len++] = *tk;
}

static void mr_classifier_set_text(struct mascara *imp,
                                    const unsigned char *str, size_t len,
                                    size_t offset_incr)
{
   struct mr_classifier *tkr = (void *)imp;

   mr_tokenizer_set_text(&tkr->tkr.base, str, len, offset_incr);
   tkr->p = str;
   tkr->pe = &str[len];

   add_token(tkr, &(struct mr_token){
      .str = (const char *)str,
      .offset = 0,
      .len = 0,
      .type = MR_UNK,
   });
   tkr->first = true;
}

static struct mr_token *fetch_tokens(struct mr_classifier *szr,
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
static void reattach_period(struct mr_classifier *szr)
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

static size_t mr_classifier_next(struct mascara *imp, struct mr_token **tks)
{
   struct mr_classifier *szr = (void *)imp;
   assert(szr->p && "text no set");

   if (!szr->first) {
      szr->tokens[0] = szr->tokens[szr->len - 2];
      szr->tokens[1] = szr->tokens[szr->len - 1];
      szr->len = 2;
   } else {
      szr->len = 1;
      szr->first = false;
   }
   return sentencize2_next(szr, tks);
}
