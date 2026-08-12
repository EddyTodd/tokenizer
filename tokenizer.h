#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tokenizer_status {
    TOKENIZER_OK = 0,
    TOKENIZER_INVALID_ARGUMENT,
    TOKENIZER_OUT_OF_MEMORY,
    TOKENIZER_SIZE_OVERFLOW
} tokenizer_status;

typedef struct tokenizer_span {
    const char *data;
    size_t length;
} tokenizer_span;

typedef struct tokenizer_iterator {
    const char *cursor;
    const char *delimiters;
    bool whitespace_mode;
} tokenizer_iterator;

typedef struct tokenizer_result {
    char **tokens;
    size_t count;
    char *storage;
} tokenizer_result;

/*
 * Initializes a zero-allocation iterator that splits on any byte present in
 * `delimiters`. The input and delimiter strings must remain alive while the
 * iterator is in use.
 */
tokenizer_status tokenizer_iterator_init(
    tokenizer_iterator *iterator,
    const char *input,
    const char *delimiters
);

/* Initializes a zero-allocation iterator that splits on C whitespace. */
tokenizer_status tokenizer_iterator_init_whitespace(
    tokenizer_iterator *iterator,
    const char *input
);

/*
 * Writes the next non-empty token view to `span` and returns true. The span
 * points into the original input and is not NUL-terminated unless the token
 * reaches the end of the input.
 */
bool tokenizer_next(tokenizer_iterator *iterator, tokenizer_span *span);

/*
 * Splits `input` on any byte present in `delimiters`.
 *
 * On success, `out->tokens` is NUL-terminated, `out->count` is the number of
 * tokens, and every token points into one owned contiguous storage block.
 */
tokenizer_status tokenizer_split(
    const char *input,
    const char *delimiters,
    tokenizer_result *out
);

/* Splits `input` on characters recognized by isspace(). */
tokenizer_status tokenizer_split_whitespace(
    const char *input,
    tokenizer_result *out
);

/* Releases all memory owned by a result and resets it to the empty state. */
void tokenizer_result_free(tokenizer_result *result);

/* Returns a stable human-readable name for a tokenizer status value. */
const char *tokenizer_status_string(tokenizer_status status);

#ifdef __cplusplus
}
#endif

#endif
