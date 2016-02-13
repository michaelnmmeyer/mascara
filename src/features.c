#include "features.h"
#include "api.h"
#include "bayes.h"
#include "lib/kabak.h"

local bool first_upper(const struct mr_token *tk)
{
   if (tk->len == 0)
      return false;

   size_t clen;
   char32_t c = kb_decode(tk->str, &clen);
   return kb_is_upper(c);
}

local char *strzcat(char *restrict buf, const char *restrict str)
{
   while (*str)
      *buf++ = *str++;
   return buf;
}

local char *ft_prefix4(char *buf, const struct mr_token *tk)
{
   size_t len = kb_offset(tk->str, tk->len, 4);
   memcpy(buf, tk->str, len);
   return &buf[len];
}

local char *ft_suffix(char *buf, const struct mr_token *tk, ptrdiff_t nr)
{
   size_t start = kb_offset(tk->str, tk->len, nr);
   size_t len = tk->len - start;
   memcpy(buf, &tk->str[start], len);
   return &buf[len];
}

#define $(n)                                                                   \
local char *ft_suffix##n(char *buf, const struct mr_token *tk)                 \
{                                                                              \
   return ft_suffix(buf, tk, -n);                                              \
}
$(2)
$(3)
$(4)
#undef $

local char *ft_case(char *buf, const struct mr_token *tk)
{
   memcpy(buf, first_upper(tk) ? "LCAP" : "LLOW", 4);
   return &buf[4];
}

local char *ft_shape(char *buf, const struct mr_token *tk)
{
   if (tk->type == MR_LATIN)
      return ft_case(buf, tk);
   return strzcat(buf, mr_type_name(tk->type));
}

#include "gen/vowel.ic"

local char *ft_mask(char *buf, const struct mr_token *tk)
{
   const char *str = tk->str;
   const size_t len = tk->len;

   for (size_t i = 0, clen; i < len; i += clen) {
      char32_t c = kb_decode(&str[i], &clen);
      switch (kb_category(c)) {
      /* Letter. */
      case KB_CATEGORY_LU:
      case KB_CATEGORY_LL:
      case KB_CATEGORY_LT:
      case KB_CATEGORY_LM:
      case KB_CATEGORY_LO:
         *buf++ = mr_is_vowel(c) ? 'V' : 'C';
         break;
      /* Digit. */
      case KB_CATEGORY_ND:
      case KB_CATEGORY_NL:
      case KB_CATEGORY_NO:
         *buf++ = 'D';
         break;
      /* Other (punctuation, probably). */
      default:
         *buf++ = 'P';
         break;
      }
   }
   return buf;
}

local char *ft_unimask(char *buf, const struct mr_token *tk)
{
   const char *str = tk->str;
   const size_t len = tk->len;

   for (size_t i = 0, clen; i < len; i += clen) {
      char32_t c = kb_decode(&str[i], &clen);
      *buf++ = kb_category(c) + '0';
   }
   return buf;
}
