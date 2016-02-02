#include "features.h"
#include "api.h"
#include "lib/utf8proc.h"

static bool first_upper(const struct mr_token *tk)
{
   const ssize_t len = tk->len < 4 ? tk->len : 4;
   int32_t c;
   if (utf8proc_iterate((const uint8_t *)tk->str, len, &c) > 0)
      return utf8proc_get_property(c)->category == UTF8PROC_CATEGORY_LU;
   return false;
}

/* A feature that won't match. */
static char *mr_ft_void(char *buf)
{
   *buf = 0xff;
   return buf + 1;
}

static char *strzcat(char *restrict buf, const char *restrict str)
{
   while (*str)
      *buf++ = *str++;
   return buf;
}

local char *mr_ft_prefix4(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t pfx_len = 0;
   size_t nr = 4;
   while (pfx_len < len && nr) {
      const size_t clen = utf8proc_utf8class[str[pfx_len]];
      if (!clen || pfx_len + clen > len)
         break;
      pfx_len += clen;
      nr--;
   }
   memcpy(buf, str, pfx_len);
   return &buf[pfx_len];
}

local char *mr_ft_suffix3(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;

   size_t nr = 3;
   while (len)
      if ((str[--len] & 0xc0) != 0x80 && !--nr)
         break;

   const size_t sfx_len = tk->len - len;
   memcpy(buf, &str[len], sfx_len);
   return &buf[sfx_len];
}

local char *mr_ft_len(char *buf, const struct mr_token *tk)
{
   static const char *const tbl[] = {
      [0] = "0",
      [1] = "1",
      [2] = "2..3",
      [3] = "2..3",
      [4] = "4..5",
      [5] = "4..5",
      [6] = "6..7",
      [7] = "6..7",
   };
   return strzcat(buf, tk->len < 8 ? tbl[tk->len] : "8..");
}

local char *mr_ft_word(char *buf, const struct mr_token *tk)
{
   if (tk->len > MAX_FEATURE_LEN)
      return mr_ft_void(buf);
   memcpy(buf, tk->str, tk->len);
   return &buf[tk->len];
}

local char *mr_ft_case(char *buf, const struct mr_token *tk)
{
   memcpy(buf, first_upper(tk) ? "LCAP" : "LLOW", 4);
   return &buf[4];
}

local char *mr_ft_shape(char *buf, const struct mr_token *tk)
{
   if (tk->type == MR_LATIN)
      return mr_ft_case(buf, tk);
   return strzcat(buf, mr_token_type_name(tk->type));
}

#include "gen/vowel.ic"

local char *mr_ft_mask(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   size_t len = tk->len;
   
   /* Technically, could match, but MAX_FEATURE_LEN is quite large already. */
   if (len > MAX_FEATURE_LEN)
      return mr_ft_void(buf);

   ssize_t clen;
   for (size_t i = 0; i < len; i += clen) {
      int32_t c;
      clen = utf8proc_iterate(&str[i], len - i, &c);
      if (clen <= 0)
         break;
      switch (utf8proc_get_property(c)->category) {
      /* Letter. */
      case UTF8PROC_CATEGORY_LU:
      case UTF8PROC_CATEGORY_LL:
      case UTF8PROC_CATEGORY_LT:
      case UTF8PROC_CATEGORY_LM:
      case UTF8PROC_CATEGORY_LO:
         *buf++ = mr_is_vowel(c) ? 'V' : 'C';
         break;
      /* Digit. */
      case UTF8PROC_CATEGORY_ND:
      case UTF8PROC_CATEGORY_NL:
      case UTF8PROC_CATEGORY_NO:
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
