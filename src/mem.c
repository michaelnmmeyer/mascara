#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

MR_LOCAL noreturn void mr_fatal(const char *msg, ...)
{
   va_list ap;

   fputs("mascara: ", stderr);
   va_start(ap, msg);
   vfprintf(stderr, msg, ap);
   va_end(ap);
   putc('\n', stderr);
   fflush(stderr);

   abort();
}

#define MR_OOM() mr_fatal("out of memory")

MR_LOCAL void *mr_malloc(size_t size)
{
   assert(size);
   void *mem = malloc(size);
   if (!mem)
      MR_OOM();
   return mem;
}

MR_LOCAL void *mr_calloc(size_t nmemb, size_t size)
{
   assert(size && nmemb);
   void *mem = calloc(nmemb, size);
   if (!mem)
      MR_OOM();
   return mem;
}

MR_LOCAL void *mr_realloc(void *mem, size_t size)
{
   assert(size);
   mem = realloc(mem, size);
   if (!mem)
      MR_OOM();
   return mem;
}
