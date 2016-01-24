#ifndef MASCARA_H
#define MASCARA_H

#define MR_VERSION "0.5"

#include <stddef.h>

/* Maximum allowed length of a sentence, in tokens. Sentences that would grow
 * larger than that are split in chunks. This is done to avoid pathological
 * cases.
 */
#define MR_MAX_SENTENCE_LEN 1000

/* See the readme file for informations about these. */
enum mr_token_type {
   MR_UNK,
   MR_LATIN,
   MR_PREFIX,
   MR_SUFFIX,
   MR_SYM,
   MR_NUM,
   MR_ABBR,
   MR_EMAIL,
   MR_URI,
   MR_PATH,
};

/* String representation of a token type. */
const char *mr_token_type_name(enum mr_token_type);

struct mascara;

/* Tokenization mode. */
enum mr_mode {
   MR_TOKEN,      /* Iterate over tokens. */
   MR_SENTENCE,   /* Iterate over sentences, where an sentence is an array of
                   * tokens. */
};

/* Returns an array containing the names of the supported languages.
 * The array is NULL-terminated.
 */
const char *const *mr_langs(void);

/* Allocates a new tokenizer.
 * If there is no implementation for the provided language name, returns NULL.
 * Available languages are "en", "fr", and "it".
 */
struct mascara *mr_alloc(const char *lang, enum mr_mode);

/* Destructor. */
void mr_dealloc(struct mascara *);

/* Returns the chosen tokenization mode. */
enum mr_mode mr_mode(const struct mascara *);

/* Sets the text to tokenize.
 * The input string must be valid UTF-8 and normalized to NFC. No internal check
 * is made to ensure that this is the case. If this isn't, the result is
 * undefined. The input string is not copied internally, and should then not be
 * deallocated until this function is called with a new string.
 */
void mr_set_text(struct mascara *, const char *str, size_t len);

struct mr_token {
   const char *str;           /* Not nul-terminated! */
   size_t len;                /* Length, in bytes. */
   size_t offset;             /* Offset from the start of the text, in bytes. */
   enum mr_token_type type;
};

/* Fetch the next token or sentence.
 * Must be called after mr_set_text().
 * The behaviour of this function depends on the chosen tokenization mode:
 * - If it is MR_TOKEN, looks for the next token in the input text. If there is
 *   one, makes the provided token pointer point to a structure filled with
 *   informations about it, and returns 1.
 * - If it is MR_SENTENCE, looks for the next sentence in the input text. If
 *   there is one, makes the provided token pointer point to an array of token
 *   structures, and returns the number of tokens in the sentence.
 * If at the end of the text, makes the provided token pointer point to NULL,
 * and returns 0.
 */
size_t mr_next(struct mascara *, struct mr_token **);

#endif
