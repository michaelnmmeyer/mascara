#ifndef MR_IMP_H
#define MR_IMP_H

#include <stdbool.h>

#define local static

/* Maximum allowed length of a sentence, in tokens. Sentences that would grow
 * larger than that are split in chunks. This is done to avoid pathological
 * cases.
 */
#define MR_MAX_SENTENCE_LEN 1000

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
