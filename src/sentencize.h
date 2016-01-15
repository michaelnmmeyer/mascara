#ifndef MR_SENTENCIZE_H
#define MR_SENTENCIZE_H

#include "imp.h"

struct mr_tokenizer_vtab;

struct mr_sentencizer {
   struct mascara base;

   const struct mr_tokenizer_vtab *vtab;
   const unsigned char *str;
   size_t offset_incr;

   const unsigned char *p;
   const unsigned char *pe;
   
   struct mr_token *tokens;
   size_t len, alloc;
};

void mr_sentencizer_init(struct mr_sentencizer *,
                         const struct mr_tokenizer_vtab *);

#endif
