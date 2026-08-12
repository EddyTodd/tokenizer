#include "tokenizer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void assert_token(const tokenizer_result *result, size_t index, const char *expected) {
    assert(result != NULL);
    assert(index < result->count);
    assert(strcmp(result->tokens[index], expected) == 0);
}

static void test_whitespace_split(void) {
    tokenizer_result result = {0};

    assert(tokenizer_split_whitespace("  alpha\tbeta\n gamma  ", &result) == TOKENIZER_OK);
    assert(result.count == 3);
    assert_token(&result, 0, "alpha");
    assert_token(&result, 1, "beta");
    assert_token(&result, 2, "gamma");
    assert(result.tokens[3] == NULL);

    tokenizer_result_free(&result);
    assert(result.tokens == NULL);
    assert(result.storage == NULL);
    assert(result.count == 0);
}

static void test_explicit_delimiters(void) {
    tokenizer_result result = {0};

    assert(tokenizer_split("alpha,beta;;gamma,delta", ",;", &result) == TOKENIZER_OK);
    assert(result.count == 4);
    assert_token(&result, 0, "alpha");
    assert_token(&result, 1, "beta");
    assert_token(&result, 2, "gamma");
    assert_token(&result, 3, "delta");

    tokenizer_result_free(&result);
}

static void test_empty_and_delimiter_only_inputs(void) {
    tokenizer_result result = {0};

    assert(tokenizer_split_whitespace("", &result) == TOKENIZER_OK);
    assert(result.count == 0);
    assert(result.tokens != NULL);
    assert(result.tokens[0] == NULL);
    tokenizer_result_free(&result);

    assert(tokenizer_split(";;;", ";", &result) == TOKENIZER_OK);
    assert(result.count == 0);
    assert(result.tokens[0] == NULL);
    tokenizer_result_free(&result);
}

static void test_empty_delimiter_set(void) {
    tokenizer_result result = {0};

    assert(tokenizer_split("entire input", "", &result) == TOKENIZER_OK);
    assert(result.count == 1);
    assert_token(&result, 0, "entire input");
    tokenizer_result_free(&result);
}

static void test_utf8_bytes_are_preserved(void) {
    tokenizer_result result = {0};

    assert(tokenizer_split_whitespace("café naïve", &result) == TOKENIZER_OK);
    assert(result.count == 2);
    assert_token(&result, 0, "café");
    assert_token(&result, 1, "naïve");
    tokenizer_result_free(&result);
}

static void test_zero_copy_iterator(void) {
    tokenizer_iterator iterator;
    tokenizer_span span;

    assert(tokenizer_iterator_init(&iterator, "one::two:::three", ":") == TOKENIZER_OK);

    assert(tokenizer_next(&iterator, &span));
    assert(span.length == 3 && strncmp(span.data, "one", span.length) == 0);
    assert(tokenizer_next(&iterator, &span));
    assert(span.length == 3 && strncmp(span.data, "two", span.length) == 0);
    assert(tokenizer_next(&iterator, &span));
    assert(span.length == 5 && strncmp(span.data, "three", span.length) == 0);
    assert(!tokenizer_next(&iterator, &span));
    assert(span.data == NULL && span.length == 0);
}

static void test_invalid_arguments(void) {
    tokenizer_iterator iterator;
    tokenizer_result result = {0};

    assert(tokenizer_split(NULL, ",", &result) == TOKENIZER_INVALID_ARGUMENT);
    assert(tokenizer_split("abc", NULL, &result) == TOKENIZER_INVALID_ARGUMENT);
    assert(tokenizer_split("abc", ",", NULL) == TOKENIZER_INVALID_ARGUMENT);
    assert(tokenizer_iterator_init(&iterator, NULL, ",") == TOKENIZER_INVALID_ARGUMENT);
    assert(tokenizer_iterator_init(&iterator, "abc", NULL) == TOKENIZER_INVALID_ARGUMENT);
    assert(tokenizer_iterator_init_whitespace(NULL, "abc") == TOKENIZER_INVALID_ARGUMENT);
    assert(strcmp(tokenizer_status_string(TOKENIZER_OK), "ok") == 0);
}

int main(void) {
    test_whitespace_split();
    test_explicit_delimiters();
    test_empty_and_delimiter_only_inputs();
    test_empty_delimiter_set();
    test_utf8_bytes_are_preserved();
    test_zero_copy_iterator();
    test_invalid_arguments();

    puts("all tokenizer tests passed");
    return 0;
}
