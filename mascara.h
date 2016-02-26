#ifndef MASCARA_H
#define MASCARA_H

#define MR_VERSION "0.10"

#include <stddef.h>

/* Location of the directory that contains model files. Should be set at
 * startup and not changed afterwards. Defaults to "models".
 */
extern const char *mr_home;

enum {
   MR_OK,      /* No error. */
   MR_EHOME,   /* Cannot find models directory. */
   MR_EOPEN,   /* Cannot open model file. */
   MR_EMAGIC,  /* Model file signature mismatch. */
   MR_EMODEL,  /* Model file is corrupt. */
   MR_EIO,     /* Cannot read model file. */
};

/* Returns a string describing an error code. */
const char *mr_strerror(int err);

/* Installs a handler for fatal errors. */
void mr_on_error(void (*handler)(const char *msg));

/* Token types. See the readme file for informations about these. */
enum mr_type {
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
const char *mr_type_name(enum mr_type);

struct mascara;

/* Tokenization modes. */
enum mr_mode {
   MR_TOKEN,      /* Iterate over tokens. */
   MR_SENTENCE,   /* Iterate over sentences (arrays of tokens). */
};

/* Returns an array containing the names of the supported languages.
 * The array is NULL-terminated and lexicographically sorted.
 */
const char *const *mr_langs(void);

/* Allocates a new tokenizer.
 * If there is no specific support for the provided language name, chooses a
 * generic tokenizer.
 * On success, makes the provided structure pointer point to an allocated
 * tokenizer, and returns MR_OK. Otherwise, makes it point to NULL, and returns
 * an error code.
 */
int mr_alloc(struct mascara **, const char *lang, enum mr_mode);

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
   enum mr_type type;
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
