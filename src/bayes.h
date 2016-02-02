#ifndef MR_BAYES_H
#define MR_BAYES_H

/* Keep that in sync with the Python code generator. */
#define MAX_FEATURE_LEN 128

struct bayes;

/* Model descriptor. */
struct bayes_config {
   const char *name;
   unsigned version;
   const char *const *features;
};

local int bayes_load(struct bayes **, const char *path,
                     const struct bayes_config *);

local void bayes_dealloc(struct bayes *);

enum {
   EOS,
   NOT_EOS,
};

/* Set priors. */
local void bayes_init(const struct bayes *, double [static 2]);

/* Feed a single feature.
 * The feature id (an integer > 0 && <= 0xff) must appear at the beginning of
 * the feature string.
 */
local void bayes_feed(const struct bayes *, double [static 2],
                      const void *feature);

#endif
