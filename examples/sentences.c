/* Shows how to split a text into sentences.
 * Input strings must be passed as argument on the command-line.
 * Outputs one sentence per line, tokens being separated with whitespace.
 */

#include <stdio.h>
#include <string.h>
#include "../mascara.h"

int main(int argc, char **argv)
{
   struct mascara *mr = mr_alloc("en", MR_SENTENCE);

   while (*++argv) {
      struct mr_token *sent;
      size_t len;
      mr_set_text(mr, *argv, strlen(*argv));
      while ((len = mr_next(mr, &sent))) {
         size_t i;
         for (i = 0; i < len; i++) {
            fwrite(sent[i].str, 1, sent[i].len, stdout);
            putchar(i + 1 == len ? '\n' : ' ');
         }
      }
   }

   mr_dealloc(mr);
   return 0;
}
