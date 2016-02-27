#ifndef KABAK_H
#define KABAK_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <uchar.h>
#include <stdarg.h>

#define KB_VERSION "0.6"

enum {
   KB_OK,      /* No error. */
   KB_FINI,    /* End of iteration (not an error). */
   KB_EUTF8,   /* Invalid UTF-8 sequence. */
};

/* Returns a string describing an error code. */
const char *kb_strerror(int err);

/* Function to call when a fatal error occurs. */
void kb_on_error(void (*handler)(const char *msg));


/*******************************************************************************
 * Dynamic buffer.
 ******************************************************************************/

struct kabak {
   char *str;        /* Zero-terminated. */
   size_t len;       /* Length in bytes. */
   size_t alloc;
};

#define KB_INIT {.str = ""}

void kb_fini(struct kabak *);

/* Append data at the end of a buffer. */
void kb_cat(struct kabak *restrict, const char *restrict str, size_t len);

/* Encodes a code point to UTF-8 and appends it to a buffer. */
void kb_catc(struct kabak *restrict, char32_t);

/* Appends a single byte to a buffer. */
void kb_catb(struct kabak *restrict, int);

/* Appends formatted data to a buffer. */
void kb_printf(struct kabak *restrict, const char *restrict fmt, ...);

/* Ensures that there's enough room for storing "size" more bytes.
 * Returns a pointer to the end of the buffer.
 */
void *kb_grow(struct kabak *, size_t size);

/* Truncation to the empty string. */
void kb_clear(struct kabak *);

/* Returns a buffer's contents as an allocated string.
 * The returned string must be freed with free(). It is zero-terminated. If
 * "len" is not NULL, it is filled with the length of the returned string.
 */
char *kb_detach(struct kabak *restrict, size_t *restrict len);


/*******************************************************************************
 * Normalization.
 ******************************************************************************/

#define KB_REPLACEMENT_CHAR 0xFFFD

enum {
   /* Compose code points. */
   KB_COMPOSE = 1 << 0,
   
   /* Decompose code points. */
   KB_DECOMPOSE = 1 << 1,
   
   /* Use compatibility mappings, with custom additional mappings. Must be
    * combined with KB_COMPOSE or KB_DECOMPOSE to be taken into account.
    */
   KB_COMPAT = 1 << 2,
   
   /* Lump some characters together. */
   KB_LUMP = 1 << 3,
   
   /* Use Unicode casefold mappings. */
   KB_CASE_FOLD = 1 << 4,

   /* Drop code points in Default_Ignorable_Code_Point. See
    * http://www.unicode.org/Public/8.0.0/ucd/DerivedCoreProperties.txt
    */
   KB_STRIP_IGNORABLE = 1 << 5,
   
   /* Drop code points in categories Cn (Other, Not Assigned) and Co (Other,
    * Private Use). Code points in Cs (Other, Surrogate) don't appear in UTF-8
    * strings.
    */
   KB_STRIP_UNKNOWN = 1 << 6,
   
   /* Drop diacritical marks (categories Mc, Me, and Mn). Must be combined with
    * KB_COMPOSE or KB_DECOMPOSE to be taken into account.
    */
   KB_STRIP_DIACRITIC = 1 << 7,

   /* NFC normalization. */
   KB_NFC = KB_COMPOSE | KB_DECOMPOSE,
   
   /* NFKC normalization (with custom additional mappings). */
   KB_NFKC = KB_COMPOSE | KB_DECOMPOSE | KB_COMPAT,
};

/* Transforms a string in some way.
 * On success, returns KB_OK, otherwise an error code. In both cases, the
 * output buffer is filled with the normalized string.
 * Bytes that cannot form valid UTF-8 sequences are replaced with REPLACEMENT
 * CHARACTER (U+FFFD). Unassigned code points and non-characters are deemed to
 * be valid. Only surrogates and code points > 0x10FFFF are considered invalid.
 * The output buffer is cleared beforehand.
 */
int kb_transform(struct kabak *restrict, const char *restrict str, size_t len,
                 unsigned opts);


/*******************************************************************************
 * I/O.
 ******************************************************************************/

/* FILE object wrapper. */
struct kb_file {
   FILE *fp;
   size_t pending;
   uint8_t backup[4];
   char32_t last;
};

/* Wraps an opened file for reading UTF-8 data from it.
 * The file can be opened in binary mode. It must not be used while the
 * kb_file structure is in use. It must be closed by the caller after use if
 * necessary. We assume that no data has been read from the file yet, and check
 * for a leading BOM.
 */
void kb_wrap(struct kb_file *restrict, FILE *restrict);

/* Reads a single line from a file and, optionally, normalizes it.
 * All possible EOL sequences are supported. The EOL sequence at the end of a
 * line, if any, is trimmed. The last line of the file is skipped if empty.
 * Returns KB_OK if a line was read, KB_FINI if at EOF, otherwise an error code.
 * I/O errors are not reported and must be checked separately. Notes above
 * kb_transform() apply here, too.
 */
int kb_get_line(struct kb_file *restrict, struct kabak *restrict,
                unsigned opts);


/*******************************************************************************
 * UTF-8.
 ******************************************************************************/

/* Decodes a single code point.
 * *clen is filled with the number of decoded bytes.
 * Typical usage:
 *
 *   for (size_t i = 0, clen; i < len; i += clen)
 *      char32_t c = kb_decode(&str[i], &clen);
 *
 * The provided UTF-8 string must be valid.
 */
char32_t kb_decode(const char *restrict str, size_t *restrict clen);

/* Safe version of kb_decode().
 *
 * Bytes that cannot form valid UTF-8 sequences are replaced with REPLACEMENT
 * CHARACTER (U+FFFD). In this case, *clen is set to 1. If at the end of the
 * string, REPLACEMENT CHARACTER is also returned, but *clen is set to 0.
 * Typical usage:
 *
 *   for (size_t i = 0, clen; i < len; i += clen) {
 *      char32_t c = kb_decode_s(&str[i], len - i, &clen);
 *      if (c == KB_REPLACEMENT_CHAR && clen == 1)
 *         puts("encoding error");
 *   }
 */
char32_t kb_decode_s(const char *restrict str, size_t len,
                     size_t *restrict clen);

/* Encodes a single code point.
 * Returns the number of bytes written.
 * The provided code point must be valid.
 */
size_t kb_encode(char buf[static 4], char32_t c);

/* Counts the number of code points in a UTF-8 string.
 * The provided UTF-8 string must be valid.
 */
size_t kb_count(const char *str, size_t len);

/* Returns the offset of the nth code point of a string.
 * If n is negative, code points are counted from the end of the string.
 * If the input string contains less than abs(n) code point, the string length
 * is returned if n is strictly positive, zero otherwise.
 * The provided UTF-8 string must be valid.
 */
size_t kb_offset(const char *str, size_t len, ptrdiff_t n);


/*******************************************************************************
 * Character classification.
 ******************************************************************************/

enum kb_category {
   KB_CATEGORY_CN,
   KB_CATEGORY_LU,
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
