#include "features.h"
#include "api.h"
#include "bayes.h"
#include "lib/utf8proc.h"

local size_t pick_char(int32_t *restrict c, const char *restrict str)
{
   const ssize_t clen = utf8proc_iterate((const uint8_t *)str, 4, c);
   assert(clen > 0);
   return clen;
}

local bool first_upper(const struct mr_token *tk)
{
   int32_t c;
   pick_char(&c, tk->str);
   return utf8proc_get_property(c)->category == UTF8PROC_CATEGORY_LU;
}

local char *strzcat(char *restrict buf, const char *restrict str)
{
   while (*str)
      *buf++ = *str++;
   return buf;
}

local char *ft_prefix4(char *buf, const struct mr_token *tk)
{
   const char *str = tk->str;
   const size_t len = tk->len;

   size_t pfx_len = 0;
   size_t nr = 4;
   while (pfx_len < len && nr) {
      const size_t clen = utf8proc_utf8class[(uint8_t)str[pfx_len]];
      assert(clen);
      pfx_len += clen;
      nr--;
   }
   memcpy(buf, str, pfx_len);
   return &buf[pfx_len];
}

local char *ft_suffix3(char *buf, const struct mr_token *tk)
{
   const char *str = tk->str;
   size_t len = tk->len;

   size_t nr = 3;
   while (len)
      if (((uint8_t)str[--len] & 0xc0) != 0x80 && !--nr)
         break;

   const size_t sfx_len = tk->len - len;
   memcpy(buf, &str[len], sfx_len);
   return &buf[sfx_len];
}

local char *ft_len(char *buf, const struct mr_token *tk)
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

local char *ft_word(char *buf, const struct mr_token *tk)
{
   memcpy(buf, tk->str, tk->len);
   return &buf[tk->len];
}

local char *ft_case(char *buf, const struct mr_token *tk)
{
   memcpy(buf, tk->len == 0 || first_upper(tk) ? "LCAP" : "LLOW", 4);
   return &buf[4];
}

local char *ft_shape(char *buf, const struct mr_token *tk)
{
   if (tk->type == MR_LATIN)
      return ft_case(buf, tk);
   return strzcat(buf, mr_token_type_name(tk->type));
}

#include "gen/vowel.ic"

local char *ft_mask(char *buf, const struct mr_token *tk)
{
   const char *str = tk->str;
   const size_t len = tk->len;

   size_t clen;
   for (size_t i = 0; i < len; i += clen) {
      int32_t c;
      clen = pick_char(&c, &str[i]);
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

/* None of the following characters are longer when normalized (if we count
 * in bytes).
 */
local char *normalize_char(char *buf, int32_t c)
{
#define $1(c) *buf++ = c; break;
#define $2(s) *buf++ = s[0]; *buf++ = s[1]; assert(!s[2]); break;

   switch (c) {
   case U'Œ':
      $2("Oe")
   case U'œ': case U'ꟹ':
      $2("oe")
   case U'Æ':
      $2("Ae")
   case U'æ': case U'ᴭ':
      $2("ae")
   case U'“': case U'”': case U'„': case U'«': case U'»': case U'‹': case U'›':
      $1('"')
   case U'‘': case U'’': case U'‚': /* Not a comma! */
      $1('\'')
   default:
      buf += utf8proc_encode_char(c, (uint8_t *)buf);
      break;
   }
   return buf;

#undef $1
#undef $2
}

local size_t normalize(char *buf, const struct mr_token *tk)
{
   const uint8_t *str = (const void *)tk->str;
   const size_t len = tk->len;
   const char *const buf_orig = buf;

   if (len > MAX_FEATURE_LEN)
      return NORM_FAILURE;

   ssize_t clen;
   for (size_t i = 0; i < len; i += clen) {
      int32_t c;
      clen = utf8proc_iterate(&str[i], len - i, &c);
      if (clen <= 0)
         return NORM_FAILURE;
      buf = normalize_char(buf, c);
   }
   return buf - buf_orig;
}
