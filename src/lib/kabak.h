#ifndef KABAK_H
#define KABAK_H

#include <stddef.h>
#include <stdbool.h>
#include <uchar.h>

#define KB_VERSION "0.2"

struct kabak {
   char *str;        /* Zero-terminated. */
   size_t len;       /* Length in bytes. */
   size_t alloc;
   unsigned flags;
};

#define KB_INIT {.str = ""}

void kb_fini(struct kabak *);

/* Append data at the end of a buffer. */
void kb_cat(struct kabak *restrict, const char *restrict str, size_t len);

/* Encodes a code point to UTF-8 and appends it to a buffer. */
void kb_catc(struct kabak *restrict, char32_t);

/* Ensures that there's enough room for storing "size" more bytes.
 * Returns a pointer to the end of the buffer.
 */
void *kb_grow(struct kabak *, size_t size);

/* Truncate to the empty string. */
void kb_clear(struct kabak *);

char *kb_detach(struct kabak *restrict, size_t *restrict len);


/*******************************************************************************
 * Normalization.
 ******************************************************************************/

enum {
   KB_MERGE = 1 << 0,      /* NFKC, with additional custom mappings. */
   KB_CASE_FOLD = 1 << 3,  /* Case folding. */
   KB_DIACR_FOLD = 1 << 4, /* Diacritic removal. */
};

/* Invalid code points are replaced with REPLACEMENT CHARACTER (U+FFFD).
 * Unassigned code points and non-characters are deemed to be valid. Only
 * surrogates and code points > 0x10FFFF are considered invalid. The output
 * buffer is cleared beforehand.
 */
void kb_transform(struct kabak *restrict, const char *restrict str, size_t len,
                  unsigned opts);


/*******************************************************************************
 * UTF-8.
 ******************************************************************************/

/* Decodes a single code point. Typical usage:
 *
 *   for (size_t i = 0, clen; i < len; i += clen)
 *      char32_t c = kb_decode(&str[i], &clen);
 */
char32_t kb_decode(const char *restrict str, size_t *restrict clen);

/* Encodes a single code point. */
size_t kb_encode(char buf[static 4], char32_t c);

/* Counts the number of code points in a UTF-8 string. */
size_t kb_count(const char *str, size_t len);

/* Returns the offset of the nth code point of a string.
 * If n is negative, code points are counted from the end of the input string.
 * If the input string contains less than abs(n) code point, the string length
 * is returned if n is strictly positive, zero otherwise.
 */
size_t kb_offset(const char *str, size_t len, ptrdiff_t n);


/*******************************************************************************
 * Character classification.
 ******************************************************************************/

enum kb_category {
   KB_CATEGORY_LU = 1,
   KB_CATEGORY_LL,
   KB_CATEGORY_LT,
   KB_CATEGORY_LM,
   KB_CATEGORY_LO,
   KB_CATEGORY_MN,
   KB_CATEGORY_MC,
   KB_CATEGORY_ME,
   KB_CATEGORY_ND,
   KB_CATEGORY_NL,
   KB_CATEGORY_NO,
   KB_CATEGORY_PC,
   KB_CATEGORY_PD,
   KB_CATEGORY_PS,
   KB_CATEGORY_PE,
   KB_CATEGORY_PI,
   KB_CATEGORY_PF,
   KB_CATEGORY_PO,
   KB_CATEGORY_SM,
   KB_CATEGORY_SC,
   KB_CATEGORY_SK,
   KB_CATEGORY_SO,
   KB_CATEGORY_ZS,
   KB_CATEGORY_ZL,
   KB_CATEGORY_ZP,
   KB_CATEGORY_CC,
   KB_CATEGORY_CF,
   KB_CATEGORY_CS,
   KB_CATEGORY_CO,
   KB_CATEGORY_CN,
};

enum kb_category kb_category(char32_t);

/* Categories L*. */
bool kb_is_letter(char32_t);

/* Category Lu. */
bool kb_is_upper(char32_t);

/* Category Ll. */
bool kb_is_lower(char32_t);

/* Categories N*. */
bool kb_is_number(char32_t);

/* Code points 0009..000D, 0085 and categories Z*. */
bool kb_is_space(char32_t);

#endif
