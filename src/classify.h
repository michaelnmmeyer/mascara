#ifndef MR_CLASSIFY_H
#define MR_CLASSIFY_H

#include "imp.h"
#include "tokenize.h"
#include "bayes.h"

struct mr_classifier {
   struct mascara base;

   struct mr_tokenizer tkr;
   struct mr_bayes *bayes;
   size_t offset_incr;

   struct mr_token *tokens;
   size_t len, alloc;
};

void mr_classifier_init(struct mr_classifier *,
                        const struct mr_tokenizer_vtab *);

#endif
