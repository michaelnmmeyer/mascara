/* Shows how to split a text into extended grapheme clusters.
 * Input strings must be passed as argument on the command-line.
 * Outputs one grapheme per line.
 */

#include <stdio.h>
#include <string.h>
#include "../mascara.h"

int main(int argc, char **argv)
{
   struct mascara *mr;
   int ret = mr_alloc(&mr, NULL, MR_GRAPHEME);
   if (ret) {
      fprintf(stderr, "cannot create tokenizer: %s\n", mr_strerror(ret));
      return 1;
   }
   
   while (*++argv) {
      struct mr_token *grapheme;
      mr_set_text(mr, *argv, strlen(*argv));
      while (mr_next(mr, &grapheme)) {
         fwrite(grapheme->str, 1, grapheme->len, stdout);
         putchar('\n');
      }
   }
   
   mr_dealloc(mr);
   return 0;
}
