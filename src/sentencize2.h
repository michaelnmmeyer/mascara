#ifndef MR_SENTENCIZE2_H
#define MR_SENTENCIZE2_H

#include <stdbool.h>
#include "imp.h"
#include "bayes.h"

typedef bool at_eos_fn(const struct mr_bayes *,
                       const struct mr_token *lhs, const struct mr_token *rhs);

struct mr_sentencizer2_config {
   const struct mr_bayes_config bayes_config;
   at_eos_fn *at_eos;
};

struct mr_sentencizer2 {
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

MR_LOCAL void mr_sentencizer2_init(struct mr_sentencizer2 *,
                                   const struct mr_tokenizer_vtab *);

#endif
