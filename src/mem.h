#ifndef MR_MEM_H
#define MR_MEM_H

#include <stddef.h>
#include <stdarg.h>
#include <stdnoreturn.h>

local noreturn void fatal(const char *, ...);

local void *mr_malloc(size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

local void *mr_calloc(size_t, size_t)
#ifdef ___GNUC__
   __attribute__((malloc))
#endif
   ;

local void *mr_realloc(void *, size_t);

#endif
