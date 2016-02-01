#ifndef MR_MEM_H
#define MR_MEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdnoreturn.h>

MR_LOCAL noreturn void mr_fatal(const char *, ...);

MR_LOCAL void *mr_malloc(size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

MR_LOCAL void *mr_calloc(size_t, size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

MR_LOCAL void *mr_realloc(void *, size_t);

#endif
