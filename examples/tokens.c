/* Shows how to split a text into tokens.
 * Input strings must be passed as argument on the command-line.
 * Outputs one token per line.
 * Error handling is omitted for brevity!
 */

#include <stdio.h>
#include <string.h>
#include "../mascara.h"

int main(int argc, char **argv)
{
   struct mascara *mr;
   int ret = mr_alloc(&mr, "en", MR_TOKEN);
   if (ret) {
      fprintf(stderr, "cannot create tokenizer: %s\n", mr_strerror(ret));
      return 1;
   }
   
   while (*++argv) {
      struct mr_token *token;
      mr_set_text(mr, *argv, strlen(*argv));
      while (mr_next(mr, &token)) {
         fwrite(token->str, 1, token->len, stdout);
         putchar('\n');
      }
   }
   
   mr_dealloc(mr);
   return 0;
}
