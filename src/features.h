#ifndef MR_FEATURES_H
#define MR_FEATURES_H

#include <limits.h>
#include "imp.h"
#include "bayes.h"

#define $(name)                                                                \
local char *mr_ft_##name(char [restrict static MAX_FEATURE_LEN + 1],           \
                         const struct mr_token *);

$(prefix4)
$(suffix3)
$(len)
$(word)
$(case)
$(shape)
$(mask)

#undef $

#define NORM_FAILURE SIZE_MAX

local size_t normalize(char [restrict static MAX_FEATURE_LEN],
                       const struct mr_token *);

#endif
