#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "api.h"
#include "imp.h"
#include "tokenize.h"
#include "sentencize.h"
#include "sentencize2.h"
#include "mem.h"

#include "gen/en_tokenize.ic"
#include "gen/fr_tokenize.ic"
#include "gen/it_tokenize.ic"
#include "gen/generic_tokenize.ic"

const char *mr_home = "models";

static const struct tokenizer_vtab *find_tokenizer(const char *name)
{
   static const struct tokenizer_vtab tbl[] = {
   #define _(name) {#name, name##_init, name##_exec},
      _(en)
      _(fr)
      _(it)
      _(generic)
   #undef _
   };

   const size_t size = sizeof tbl / sizeof *tbl - 1;
   for (size_t i = 0; i < size; i++)
      if (!strcmp(tbl[i].name, name))
         return &tbl[i];

   return &tbl[size];
}

const char *const *mr_langs(void)
{
   static const char *const lst[] = {"en", "fr", "it", "generic", NULL};
   return lst;
}

const char *mr_strerror(int err)
{
   static const char *const tbl[] = {
      [MR_OK] = "no error",
      [MR_EHOME] = "cannot find models directory",
      [MR_EOPEN] = "cannot open model file",
      [MR_EMAGIC] = "model file signature mismatch",
      [MR_EMODEL] = "model file is corrupt",
      [MR_EIO] = "cannot read model file",
   };

   if (err >= 0 && (size_t)err < sizeof tbl / sizeof *tbl)
      return tbl[err];
   return "unknown error";
}

const char *mr_token_type_name(enum mr_token_type t)
{
   static const char *const tbl[] = {
      [MR_UNK] = "UNK",
      [MR_LATIN] = "LATIN",
      [MR_PREFIX] = "PREFIX",
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

int mr_alloc(struct mascara **mrp, const char *lang, enum mr_mode mode)
{
   const struct tokenizer_vtab *tk = find_tokenizer(lang);

   switch (mode) {
   case MR_TOKEN: {
      struct tokenizer *mr = mr_malloc(sizeof *mr);
      tokenizer_init(mr, tk);
      *mrp = &mr->base;
      break;
   }
   case MR_SENTENCE: {
      const struct sentencizer2_config *cfg = find_sentencizer2(lang);
      if (cfg) {
         struct sentencizer2 *mr = mr_malloc(sizeof *mr);
         int ret = sentencizer2_init(mr, tk, cfg);
         if (ret) {
            /* Could fall back to the FSM, but hiding this kind of error is not
             * a good idea.
             */
            free(mr);
            *mrp = NULL;
            return ret;
         }
         *mrp = &mr->base;
      } else {
         struct sentencizer *mr = mr_malloc(sizeof *mr);
         sentencizer_init(mr, tk);
         *mrp = &mr->base;
      }      
      break;
   }
   default:
      fatal("bad tokenization mode: %u", mode);
   }
   return MR_OK;
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
