#ifndef MR_BAYES_H
#define MR_BAYES_H

struct mr_bayes;

struct mr_bayes_config {
   const char *signature;
   const char *const *features;
};

int mr_bayes_load(struct mr_bayes **, const char *path,
                  const struct mr_bayes_config *);

void mr_bayes_dealloc(struct mr_bayes *);

enum {
   MR_EOS,
   MR_NOT_EOS,
};

void mr_bayes_init(const struct mr_bayes *, double [static 2]);
void mr_bayes_feed(const struct mr_bayes *, double [static 2], const void *ft);

#endif
