/* Tokenizes strings passed as arguments on the command-line. */

#include <stdio.h>
#include <string.h>
#include "mascara.h"

int main(int argc, char **argv)
{
   struct mascara *mr = mr_alloc("en");
   
   while (*++argv) {
      struct mr_token tk;
      mr_set_text(mr, *argv, strlen(*argv));
      while (mr_next_token(mr, &tk))
         printf("%.*s\n", (int)tk.len, tk.str);
   }
   
   mr_dealloc(mr);
   return 0;
}
