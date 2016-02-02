#ifndef MR_SENTENCIZE2_H
#define MR_SENTENCIZE2_H

#include <stdbool.h>
#include "imp.h"
#include "bayes.h"

typedef bool at_eos_fn(const struct bayes *,
                       const struct mr_token *lhs, const struct mr_token *rhs);

struct sentencizer2_config {
   const struct bayes_config bayes_config;
   at_eos_fn *at_eos;
};

struct sentencizer2 {
   struct mascara base;

   /* EOS classifier. */
   struct bayes *bayes;
   at_eos_fn *at_eos;
   
   struct tokenizer tkr;
   const unsigned char *p, *pe;

   /* Current sentence. We store at 0 the last token of the previous sentence,
    * or a dummy token if at the beginning of the text. The current sentence
    * starts at 1, and ends at -2 included. The token at -1 is the first token
    * of the next sentence, or a dummy one if at the end of the text.
    */
   struct sentence sent;
};

local const struct sentencizer2_config *find_sentencizer2(const char *lang);

local int sentencizer2_init(struct sentencizer2 *,
                            const struct tokenizer_vtab *,
                            const struct sentencizer2_config *);

#endif
