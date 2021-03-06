#ifndef MR_FEATURES_H
#define MR_FEATURES_H

#include <limits.h>
#include "imp.h"
#include "bayes.h"

/* Tokens must be normalized before one of the feature extraction functions is
 * called. In these, we assume that the token is valid UTF-8 and that its length
 * doesn't exceed MAX_FEATURE_LEN.
 */

#define $(name)                                                                \
local char *ft_##name(char [restrict static MAX_FEATURE_LEN + 1],              \
                      const struct mr_token *);

$(prefix4)
$(suffix2)
$(suffix3)
$(case)
$(shape)
$(mask)
$(unimask)

#undef $

#endif
