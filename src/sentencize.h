#ifndef MR_SENTENCIZE_H
#define MR_SENTENCIZE_H

#include "imp.h"

struct tokenizer_vtab;

struct sentencizer {
   struct mascara base;

   const struct tokenizer_vtab *vtab;
   const unsigned char *str;
   size_t offset_incr;

   const unsigned char *p;
   const unsigned char *pe;

   struct sentence sent;
};

local void sentencizer_init(struct sentencizer *,
                            const struct tokenizer_vtab *);

#endif
