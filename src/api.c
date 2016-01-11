#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "api.h"

static void mr_log(const char *msg, ...)
{
   va_list ap;

   fputs("mascara: ", stderr);
   va_start(ap, msg);
   vfprintf(stderr, msg, ap);
   va_end(ap);
   putc('\n', stderr);
   fflush(stderr);
}

struct tokenizer {
   const char *name;
   void (*init)(struct mascara *);
   void (*exec)(struct mascara *, struct mr_token *, int);
};

struct mascara {
   const struct tokenizer *tokenizer;

   /* Beginning of the string. Needed to compute offsets. */
   const unsigned char *str;
   
   /* Ragel variables. */
   const unsigned char *p, *pe;
   const unsigned char *ts, *te;
   const unsigned char *eof;
   int cs, act;
   
   size_t suffix_len;
};

#define MR_PARA_BREAK 666

#include "gen/tokenize_en.ic"
#include "gen/tokenize_fr.ic"

static const struct tokenizer *find_tokenizer(const char *name)
{
   static const struct tokenizer tbl[] = {
   #define _(name) {#name, name##_init, name##_exec},
      _(en)
      _(fr)
   #undef _
      {NULL, 0, 0}
   };
   
   for (const struct tokenizer *tk = tbl; tk->name; tk++)
      if (!strcmp(tk->name, name))
         return tk;
   return NULL;
}

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

   if (t >= 0 && (size_t)t < sizeof tbl / sizeof *tbl)
      return tbl[t];
   return *tbl;
}

struct mascara *mr_alloc(const char *name)
{
   const struct tokenizer *tk = find_tokenizer(name);
   if (!tk)
      return NULL;
   
   struct mascara *mr = calloc(1, sizeof *mr);
   mr->tokenizer = tk;
   return mr;
}

void mr_dealloc(struct mascara *mr)
{
   free(mr);
}

void mr_set_text(struct mascara *mr, const char *str, size_t len)
{
   const unsigned char *s = (const unsigned char *)str;

   mr->str = s;
   mr->p = s;
   mr->pe = &s[len];
   mr->eof = mr->pe;
   mr->tokenizer->init(mr);
   
   /* Skip the leading BOM, if any. Preserver correct token offsets. */
   if (len >= 3 && s[0] == 0xef && s[1] == 0xbb && s[2] == 0xbf)
      mr->p += 3;
}

static int fetch_token(struct mascara *mr, struct mr_token *tk, int emit_para)
{   
   /* Pending suffix? */
   if (mr->suffix_len) {
      tk->type = MR_SUFFIX;
      tk->str = (const char *)(mr->te - mr->suffix_len);
      tk->len = mr->suffix_len;
      tk->offset = tk->str - (const char *)mr->str;
      mr->suffix_len = 0;
      return 1;
   }
   
   /* Common case. */
   tk->str = NULL;
   mr->tokenizer->exec(mr, tk, emit_para);
   return tk->str != NULL;
}

int mr_next_token(struct mascara *mr, struct mr_token *tk)
{
   /* Text not set yet? */
   if (!mr->str) {
      mr_log("bad API usage: text not set yet");
      return 0;
   }

   return fetch_token(mr, tk, 0);
}

/* Define a maximum sentence length to avoid pathological cases. */
#define MR_MAX_SENTENCE_LEN 333

void mr_sentence_fini(struct mr_sentence *st)
{
   free(st->tokens);
}

static void add_token(struct mr_sentence *st, const struct mr_token *tk)
{
   if (st->len == st->alloc) {
      st->alloc = st->alloc * 2 + 4;
      st->tokens = realloc(st->tokens, st->alloc * sizeof *st->tokens);
   }
   st->tokens[st->len++] = *tk;
}

int mr_next_sentence(struct mascara *mr, struct mr_sentence *st)
{
   /* Text not set yet? */
   if (!mr->str) {
      mr_log("bad API usage: text not set yet");
      return 0;
   }

   struct mr_token tk;
   
   st->len = 0;
   while (fetch_token(mr, &tk, 1) && st->len < MR_MAX_SENTENCE_LEN) {
      if (tk.type == MR_PARA_BREAK) {
         /* Only emit a sentence if we have gathered one already. */
         if (st->len)
            return 1;
         continue;
      }
      add_token(st, &tk);
      if (tk.type == MR_SYM && tk.len == 1) {
         switch (*tk.str) {
         case '.': case '?': case '!':
            return 1;
         }
      }
   }
   return st->len > 0;
}
