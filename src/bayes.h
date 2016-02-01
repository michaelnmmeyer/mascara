#ifndef MR_BAYES_H
#define MR_BAYES_H

/* Keep that in sync with the Python code generator. */
#define MAX_FEATURE_LEN 128

struct mr_bayes;

/* Model descriptor. */
struct mr_bayes_config {
   const char *name;
   unsigned version;
   const char *const *features;
};

MR_LOCAL int mr_bayes_load(struct mr_bayes **, const char *path,
                           const struct mr_bayes_config *);

MR_LOCAL void mr_bayes_dealloc(struct mr_bayes *);

enum {
   MR_EOS,
   MR_NOT_EOS,
};

/* Set priors. */
MR_LOCAL void mr_bayes_init(const struct mr_bayes *, double [static 2]);

/* Feed a single feature.
 * The feature id (an integer > 0 && <= 0xff) must appear at the beginning of
 * the feature string.
 */
MR_LOCAL void mr_bayes_feed(const struct mr_bayes *, double [static 2],
                            const void *feature);

#endif
