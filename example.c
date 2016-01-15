/* Tokenizes strings passed as arguments on the command-line.
 * Outputs one token per line, an empty line to signal the end of a sentence.
 */

#include <stdio.h>
#include <string.h>
#include "mascara.h"

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
            putchar('\n');
         }
         putchar('\n');
      }
   }
   
   mr_dealloc(mr);
   return 0;
}
