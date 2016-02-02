#ifndef MR_TOKENIZE_H
#define MR_TOKENIZE_H

#include "imp.h"

struct mr_token;
struct tokenizer;

struct tokenizer_vtab {
   const char *name;
   void (*init)(struct tokenizer *);
   void (*exec)(struct tokenizer *, struct mr_token *);
};

struct tokenizer {
   struct mascara base;

   const struct tokenizer_vtab *vtab;
   const unsigned char *str;
   size_t offset_incr;

   /* Ragel variables. */
   const unsigned char *p, *pe;
   const unsigned char *ts, *te;
   const unsigned char *eof;
   int cs, act;

   /* > 0 if there is a pending suffix waiting to be emitted. */
   size_t suffix_len;

   struct mr_token token;
};

extern const struct mr_imp mr_tokenizer_imp;

local void tokenizer_init(struct tokenizer *, const struct tokenizer_vtab *);

local void tokenizer_set_text(struct mascara *,
                              const unsigned char *str, size_t len,
                              size_t offset_incr);

local size_t tokenizer_next(struct mascara *, struct mr_token **);

#endif
