#include <stdlib.h>
#include <stdio.h>
#include "mem.h"

local void (*mr_error_handler)(const char *msg);

void mr_on_error(void (*handler)(const char *msg))
{
   kb_on_error(handler);
   mr_error_handler = handler;
}

local noreturn void fatal(const char *msg, ...)
{
   if (mr_error_handler) {
      char str[1024];
      va_list ap;
      va_start(ap, msg);
      vsnprintf(str, sizeof str, msg, ap);
      va_end(ap);
      mr_error_handler(str);
   }   
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
