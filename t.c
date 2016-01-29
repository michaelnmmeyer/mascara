#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mascara.h"
#include "src/bayes.h"

struct mr_classifier_config {
   const char *name;
   const struct mr_bayes_config cfg;
   bool (*eos)(const struct mr_bayes *mdl, const struct mr_token *, const struct mr_token *);
};

static const char *const fr_sequoia_features[] = {
   "l_shape+l_len",
   "l_word+r_shape",
   NULL
};

static const struct mr_bayes_config fr_sequoia_config = {
   .signature = "fr_sequoia 1",
   .features = fr_sequoia_features,
};

#define MAX_FEATURE_LEN 255

extern char *ft_len(char *buf, const struct mr_token *tk);
extern char *ft_word(char *buf, const struct mr_token *tk);
extern char *ft_shape(char *buf, const struct mr_token *tk);

bool eos(const struct mr_bayes *mdl, const struct mr_token *left, const struct mr_token *right)
{
   double vec[2];
   char stack[MAX_FEATURE_LEN + 1], *buf;

   mr_bayes_init(mdl, vec);

   buf = stack;
   *buf++ = 1;
   buf = ft_shape(buf, left);
   *buf++ = '+';
   buf = ft_len(buf, left);
   *buf = '\0';
   mr_bayes_feed(mdl, vec, stack);

   buf = stack;
   *buf++ = 2;
   buf = ft_word(buf, left);
   *buf++ = '+';
   buf = ft_shape(buf, right);
   *buf = '\0';
   mr_bayes_feed(mdl, vec, stack);

   return vec[MR_EOS] >= vec[MR_NOT_EOS];
}

static struct mr_token make_token(char *s)
{
   struct mr_token *tk;
   struct mascara *mr = mr_alloc("fr", MR_TOKEN);

   mr_set_text(mr, s, strlen(s));
   if (!mr_next(mr, &tk))
      abort();

   struct mr_token x = *tk;
   mr_dealloc(mr);
   return x;
}

int main(int argc, char **argv)
{
   if (argc != 3)
      abort();

   const char *path = "models/fr_sequoia.mdl";

   struct mr_bayes *mdl;
   int ret = mr_bayes_load(&mdl, path, &fr_sequoia_config);
   if (ret) {
      printf("%s\n", mr_strerror(ret));
      return 1;
   }

   struct mr_token left = make_token(argv[1]);
   struct mr_token right = make_token(argv[2]);

   printf("%s\n", eos(mdl, &left, &right) ? "EOS" : "!EOS");


   mr_bayes_dealloc(mdl);
}
