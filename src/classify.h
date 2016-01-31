#ifndef MR_CLASSIFY_H
#define MR_CLASSIFY_H

#include "imp.h"
#include "bayes.h"

struct mr_classifier {
   struct mascara base;
   
   const struct mr_tokenizer_vtab *vtab;

   struct mr_classifier_config;
   const struct mr_classifier_config *cfg;
   struct mr_bayes *bayes;
   
   size_t offset_incr;
   const unsigned char *p, *pe;

   struct mr_token *tokens;
   size_t len, alloc;
};

void mr_classifier_init(struct mr_classifier *, const struct mr_tokenizer_vtab *);

#endif
