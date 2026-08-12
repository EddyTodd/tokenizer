#include "tokenizer.h"

#include <stdio.h>

int main(void) {
    const char *input = "compile, test; benchmark";
    tokenizer_result result = {0};
    tokenizer_status status = tokenizer_split(input, ",; ", &result);
    size_t i;

    if (status != TOKENIZER_OK) {
        fprintf(stderr, "tokenize failed: %s\n", tokenizer_status_string(status));
        return 1;
    }

    for (i = 0; i < result.count; ++i) {
        printf("%zu: %s\n", i, result.tokens[i]);
    }

    tokenizer_result_free(&result);
    return 0;
}
