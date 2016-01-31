#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mascara.h"
#include "src/bayes.h"



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
