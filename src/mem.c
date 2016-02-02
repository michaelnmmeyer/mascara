#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

local noreturn void fatal(const char *msg, ...)
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

#define MR_OOM() fatal("out of memory")

local void *mr_malloc(size_t size)
{
   assert(size);
   void *mem = malloc(size);
   if (!mem)
      MR_OOM();
   return mem;
}

local void *mr_calloc(size_t nmemb, size_t size)
{
   assert(size && nmemb);
   void *mem = calloc(nmemb, size);
   if (!mem)
      MR_OOM();
   return mem;
}

local void *mr_realloc(void *mem, size_t size)
{
   assert(size);
   mem = realloc(mem, size);
   if (!mem)
      MR_OOM();
   return mem;
}
