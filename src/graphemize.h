#ifndef MR_GRAPHEMIZE_H
#define MR_GRAPHEMIZE_H

#include "imp.h"

struct mr_token;

struct graphemizer {
   struct mascara base;

   const unsigned char *str;
   size_t offset_incr;

   const unsigned char *p, *pe;
   const unsigned char *ts, *te;
   const unsigned char *eof;
   int cs, act;

   struct mr_token token;
};

extern const struct mr_imp mr_graphemizer_imp;

local void graphemizer_init(struct graphemizer *);

#endif
