#ifndef MR_IMP_H
#define MR_IMP_H

#include <stdbool.h>

#define local static

struct mascara;
struct mr_token;

struct mr_imp {
   void (*set_text)(struct mascara *, const unsigned char *, size_t, size_t);
   size_t (*next)(struct mascara *, struct mr_token **);
   void (*fini)(struct mascara *);  /* Can be = 0. */
};

struct mascara {
   const struct mr_imp *imp;
};

local bool can_reattach_period(const struct mr_token *lhs,
                               const struct mr_token *period);

struct sentence {
   struct mr_token *tokens;
   size_t len, alloc;
};

local void sentence_add(struct sentence *sent, const struct mr_token *tk);
local void sentence_clear(struct sentence *sent);

#endif
