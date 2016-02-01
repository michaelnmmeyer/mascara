#ifndef MR_MEM_H
#define MR_MEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdnoreturn.h>

noreturn void mr_fatal(const char *, ...);

void *mr_malloc(size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

void *mr_calloc(size_t, size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

void *mr_realloc(void *, size_t);

#endif
