#ifndef MR_CLASSIFY_H
#define MR_CLASSIFY_H

#include <stdbool.h>
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
   const unsigned char *p, *pe;

   /* Current sentence. We store at 0 the last token of the previous sentence,
    * or a dummy token if at the beginning of the text. The current sentence
    * starts at 1, and ends at -2 included. The token at -1 is the first token
    * of the next sentence, or a dummy one if at the end of the text.
    */
   struct mr_token *tokens;
   size_t len, alloc;
   
   /* Whether we're at the start of the text. */
   bool first;
};

void mr_classifier_init(struct mr_classifier *, const struct mr_tokenizer_vtab *);

#endif
