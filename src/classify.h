#ifndef MR_CLASSIFY_H
#define MR_CLASSIFY_H

#include "imp.h"
#include "bayes.h"

typedef bool at_eos_fn(const struct mr_bayes *,
                       const struct mr_token *lhs, const struct mr_token *rhs);

struct mr_classifier {
   struct mascara base;
   
   /* EOS classifier. */
   struct mr_bayes *bayes;
   at_eos_fn *at_eos;
   
   struct mr_tokenizer tkr;

   /* Ragel variables. */
   const unsigned char *p, *pe;

   struct mr_token *tokens;
   size_t len, alloc;
   
   bool first;
};

void mr_classifier_init(struct mr_classifier *, const struct mr_tokenizer_vtab *);

#endif
