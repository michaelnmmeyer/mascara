#ifndef MASCARA_H
#define MASCARA_H

#define MR_VERSION "0.3"

#include <stddef.h>

enum mr_token_type {
   MR_UNK,
   MR_LATIN,
   MR_ELISION,
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

/* Allocates a new tokenizer.
 * If there is no implementation for the provided language name, returns NULL.
 * Available languages are "en" and "fr".
 */
struct mascara *mr_alloc(const char *lang);
void mr_dealloc(struct mascara *);

/* Sets the text to tokenize.
 * The input string must be valid UTF-8 and normalized to NFC. No internal check
 * is made to ensure that this is the case. If this isn't, the result is
 * undefined. The input string is not copied internally, and should then not be
 * deallocated until this function is called with a new string.
 */
void mr_set_text(struct mascara *, const char *str, size_t len);

struct mr_token {
   const char *str;
   size_t len;
   size_t offset;
   enum mr_token_type type;
};

/* Fetch the next token.
 * Must be called after mr_set_text().
 * If there is a next token, fills the provided token structure with
 * informations about it, and returns 1. Otherwise, returns 0. In this case, the
 * token structure contents is undefined.
 */
int mr_next_token(struct mascara *, struct mr_token *);

struct mr_sentence {
   struct mr_token *tokens;
   size_t len;
   size_t alloc;
};

/* Initializer and destructor. */
#define MR_SENTENCE_INIT {.tokens = 0}
void mr_sentence_fini(struct mr_sentence *);

/* Fetch the next sentence. */
int mr_next_sentence(struct mascara *, struct mr_sentence *);

#endif
