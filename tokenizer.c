#include "tokenizer.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool is_delimiter(char c, const char *delimiters, bool whitespace_mode) {
    if (whitespace_mode) {
        return isspace((unsigned char)c) != 0;
    }

    return strchr(delimiters, (unsigned char)c) != NULL;
}

static void reset_result(tokenizer_result *result) {
    result->tokens = NULL;
    result->count = 0;
    result->storage = NULL;
}

static size_t count_tokens(const char *input, const char *delimiters, bool whitespace_mode) {
    size_t count = 0;
    const char *cursor = input;

    while (*cursor != '\0') {
        while (*cursor != '\0' && is_delimiter(*cursor, delimiters, whitespace_mode)) {
            ++cursor;
        }

        if (*cursor == '\0') {
            break;
        }

        ++count;
        while (*cursor != '\0' && !is_delimiter(*cursor, delimiters, whitespace_mode)) {
            ++cursor;
        }
    }

    return count;
}

static tokenizer_status split_impl(
    const char *input,
    const char *delimiters,
    bool whitespace_mode,
    tokenizer_result *out
) {
    size_t count;
    size_t input_size;
    char **tokens;
    char *storage;
    char *cursor;
    size_t index = 0;

    if (out == NULL) {
        return TOKENIZER_INVALID_ARGUMENT;
    }
    reset_result(out);

    if (input == NULL || (!whitespace_mode && delimiters == NULL)) {
        return TOKENIZER_INVALID_ARGUMENT;
    }

    count = count_tokens(input, delimiters, whitespace_mode);
    if (count > (SIZE_MAX / sizeof(*tokens)) - 1U) {
        return TOKENIZER_SIZE_OVERFLOW;
    }

    input_size = strlen(input) + 1U;
    tokens = calloc(count + 1U, sizeof(*tokens));
    if (tokens == NULL) {
        return TOKENIZER_OUT_OF_MEMORY;
    }

    storage = malloc(input_size);
    if (storage == NULL) {
        free(tokens);
        return TOKENIZER_OUT_OF_MEMORY;
    }
    memcpy(storage, input, input_size);

    cursor = storage;
    while (*cursor != '\0') {
        while (*cursor != '\0' && is_delimiter(*cursor, delimiters, whitespace_mode)) {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') {
            break;
        }

        tokens[index++] = cursor;
        while (*cursor != '\0' && !is_delimiter(*cursor, delimiters, whitespace_mode)) {
            ++cursor;
        }
    }

    out->tokens = tokens;
    out->count = index;
    out->storage = storage;
    return TOKENIZER_OK;
}

tokenizer_status tokenizer_iterator_init(
    tokenizer_iterator *iterator,
    const char *input,
    const char *delimiters
) {
    if (iterator == NULL || input == NULL || delimiters == NULL) {
        return TOKENIZER_INVALID_ARGUMENT;
    }

    iterator->cursor = input;
    iterator->delimiters = delimiters;
    iterator->whitespace_mode = false;
    return TOKENIZER_OK;
}

tokenizer_status tokenizer_iterator_init_whitespace(
    tokenizer_iterator *iterator,
    const char *input
) {
    if (iterator == NULL || input == NULL) {
        return TOKENIZER_INVALID_ARGUMENT;
    }

    iterator->cursor = input;
    iterator->delimiters = NULL;
    iterator->whitespace_mode = true;
    return TOKENIZER_OK;
}

bool tokenizer_next(tokenizer_iterator *iterator, tokenizer_span *span) {
    const char *start;
    const char *cursor;

    if (iterator == NULL || span == NULL || iterator->cursor == NULL) {
        return false;
    }

    cursor = iterator->cursor;
    while (*cursor != '\0' &&
           is_delimiter(*cursor, iterator->delimiters, iterator->whitespace_mode)) {
        ++cursor;
    }

    if (*cursor == '\0') {
        iterator->cursor = cursor;
        span->data = NULL;
        span->length = 0;
        return false;
    }

    start = cursor;
    while (*cursor != '\0' &&
           !is_delimiter(*cursor, iterator->delimiters, iterator->whitespace_mode)) {
        ++cursor;
    }

    iterator->cursor = cursor;
    span->data = start;
    span->length = (size_t)(cursor - start);
    return true;
}

tokenizer_status tokenizer_split(
    const char *input,
    const char *delimiters,
    tokenizer_result *out
) {
    return split_impl(input, delimiters, false, out);
}

tokenizer_status tokenizer_split_whitespace(
    const char *input,
    tokenizer_result *out
) {
    return split_impl(input, NULL, true, out);
}

void tokenizer_result_free(tokenizer_result *result) {
    if (result == NULL) {
        return;
    }

    free(result->storage);
    free(result->tokens);
    reset_result(result);
}

const char *tokenizer_status_string(tokenizer_status status) {
    switch (status) {
        case TOKENIZER_OK:
            return "ok";
        case TOKENIZER_INVALID_ARGUMENT:
            return "invalid argument";
        case TOKENIZER_OUT_OF_MEMORY:
            return "out of memory";
        case TOKENIZER_SIZE_OVERFLOW:
            return "size overflow";
        default:
            return "unknown status";
    }
}
