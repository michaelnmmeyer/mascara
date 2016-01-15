#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "api.h"
#include "imp.h"
#include "tokenize.h"
#include "sentencize.h"

const char *mr_token_type_name(enum mr_token_type t)
{
   static const char *const tbl[] = {
      [MR_UNK] = "UNK",
      [MR_LATIN] = "LATIN",
      [MR_ELISION] = "ELISION",
      [MR_SUFFIX] = "SUFFIX",
      [MR_SYM] = "SYM",
      [MR_NUM] = "NUM",
      [MR_ABBR] = "ABBR",
      [MR_EMAIL] = "EMAIL",
      [MR_URI] = "URI",
      [MR_PATH] = "PATH",
   };

   if (t < sizeof tbl / sizeof *tbl)
      return tbl[t];
   return *tbl;
}

struct mascara *mr_alloc(const char *lang, enum mr_mode mode)
{
   const struct mr_tokenizer_vtab *tk = mr_find_tokenizer(lang);
   if (!tk)
      return NULL;

   switch (mode) {
   case MR_TOKEN: {
      struct mr_tokenizer *mr = malloc(sizeof *mr);
      mr_tokenizer_init(mr, tk);
      return &mr->base;
   }
   case MR_SENTENCE: {
      struct mr_sentencizer *mr = malloc(sizeof *mr);
      mr_sentencizer_init(mr, tk);
      return &mr->base;
   }
   }
   return NULL;
}

enum mr_mode mr_mode(const struct mascara *mr)
{
   return mr->imp == &mr_tokenizer_imp ? MR_TOKEN : MR_SENTENCE;
}

void mr_set_text(struct mascara *mr, const char *str, size_t len)
{
   const unsigned char *s = (const unsigned char *)str;

   /* Skip the leading BOM, if any. Preserve correct token offsets. */
   size_t incr = 0;
   if (len >= 3 && s[0] == 0xef && s[1] == 0xbb && s[2] == 0xbf) {
      incr = 3;
      s += incr;
      len -= incr;
   }
   mr->imp->set_text(mr, s, len, incr);
}

size_t mr_next(struct mascara *mr, struct mr_token **tk)
{
   return mr->imp->next(mr, tk);
}

void mr_dealloc(struct mascara *mr)
{
   void (*fini)(struct mascara *) = mr->imp->fini;
   if (fini)
      fini(mr);

   free(mr);
}
