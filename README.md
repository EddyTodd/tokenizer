# tokenizer

A small C11 tokenization library for splitting byte strings safely and predictably.

The original version of this repository was a compact whitespace parser written in 2020. This revision keeps the project intentionally small while giving it a defined API, explicit ownership, edge-case coverage, zero-allocation iteration, configurable delimiters, and local validation.

## Why this library exists

C's standard library provides low-level string functions, but tokenization often ends up relying on `strtok`, ad-hoc pointer arithmetic, or repeated small allocations. Those approaches can mutate caller data, hide global state, complicate nested parsing, or make ownership unclear.

This library provides two complementary interfaces:

- **`tokenizer_split*`** — materialize NUL-terminated tokens using one pointer-table allocation and one contiguous storage allocation.
- **`tokenizer_iterator*`** — iterate over non-owning spans with zero allocations and without modifying the input.

The scope is deliberately narrow: this is a byte-string tokenizer, not a Unicode word-break engine or a programming-language lexer.

## Features

- C11, no external dependencies
- input is accepted as `const char *`
- whitespace tokenization via `isspace()`
- caller-defined delimiter sets
- zero-allocation iterator API
- materialized results with explicit token count
- NUL-terminated token pointer array for convenient C iteration
- two allocations total for materialized tokenization, independent of token count
- deterministic handling of empty strings and repeated delimiters
- UTF-8 bytes are preserved transparently when delimiters are ASCII/byte-based
- explicit error/status reporting
- AddressSanitizer and UndefinedBehaviorSanitizer validation target
- focused unit tests and example program

## Quick start

```c
#include "tokenizer.h"

#include <stdio.h>

int main(void) {
    tokenizer_result result = {0};
    tokenizer_status status = tokenizer_split("alpha,beta;gamma", ",;", &result);

    if (status != TOKENIZER_OK) {
        fprintf(stderr, "tokenize failed: %s\n", tokenizer_status_string(status));
        return 1;
    }

    for (size_t i = 0; i < result.count; ++i) {
        printf("%s\n", result.tokens[i]);
    }

    tokenizer_result_free(&result);
    return 0;
}
```

Build and run the included example:

```sh
make run-example
```

## Zero-allocation iteration

When token strings do not need to outlive the input, the iterator avoids all allocation and copying:

```c
tokenizer_iterator iterator;
tokenizer_span span;

tokenizer_iterator_init_whitespace(&iterator, "one two three");

while (tokenizer_next(&iterator, &span)) {
    printf("%.*s\n", (int)span.length, span.data);
}
```

A `tokenizer_span` is a view into the original input. It is not guaranteed to be NUL-terminated, so consumers should use its explicit length.

## API

### Materialized tokenization

```c
tokenizer_status tokenizer_split(
    const char *input,
    const char *delimiters,
    tokenizer_result *out
);

tokenizer_status tokenizer_split_whitespace(
    const char *input,
    tokenizer_result *out
);

void tokenizer_result_free(tokenizer_result *result);
```

`tokenizer_split()` treats every byte contained in `delimiters` as a separator. Runs of delimiters do not create empty tokens. An empty delimiter string therefore returns the complete non-empty input as one token.

`tokenizer_split_whitespace()` uses the C library's `isspace()` classification. It correctly handles spaces, tabs, newlines, carriage returns, form feeds, and vertical tabs under the active C locale.

On success, `tokenizer_result` contains:

```c
typedef struct tokenizer_result {
    char **tokens;   /* NUL-terminated pointer array */
    size_t count;    /* number of tokens */
    char *storage;   /* owned contiguous token storage */
} tokenizer_result;
```

Call `tokenizer_result_free()` exactly once when the result is no longer needed. The function resets the structure to an empty state and accepts `NULL`.

### Iterator tokenization

```c
tokenizer_status tokenizer_iterator_init(
    tokenizer_iterator *iterator,
    const char *input,
    const char *delimiters
);

tokenizer_status tokenizer_iterator_init_whitespace(
    tokenizer_iterator *iterator,
    const char *input
);

bool tokenizer_next(tokenizer_iterator *iterator, tokenizer_span *span);
```

The iterator never mutates or copies the input. Both the input and custom delimiter string must remain alive for the lifetime of the iterator.

### Status codes

```c
TOKENIZER_OK
TOKENIZER_INVALID_ARGUMENT
TOKENIZER_OUT_OF_MEMORY
TOKENIZER_SIZE_OVERFLOW
```

Use `tokenizer_status_string()` for a stable human-readable description.

## Semantics

This project tokenizes **bytes**, not Unicode grapheme clusters or linguistic words. For custom-delimiter mode, delimiter membership is byte-based. UTF-8 content that does not contain a delimiter byte is copied unchanged.

The whitespace API delegates classification to `isspace((unsigned char)c)`, avoiding the undefined behavior that results from passing a negative signed `char` directly to `<ctype.h>` functions.

Repeated delimiters are collapsed conceptually:

```text
input:       alpha,,,beta;;gamma
delimiters: ,;
result:      [alpha] [beta] [gamma]
```

Leading and trailing delimiters are ignored, and an empty or delimiter-only input produces zero tokens.

## Complexity

For an input of `n` bytes and a delimiter set of `d` bytes:

- whitespace splitting: **O(n)** time
- current custom-delimiter splitting: **O(n × d)** worst case because delimiter membership uses `strchr`
- iterator auxiliary memory: **O(1)**
- materialized splitting auxiliary memory: **O(n + k)** for copied input and `k + 1` token pointers

The custom delimiter check is intentionally simple for this small library. A future benchmark-driven revision can evaluate a 256-entry lookup table or bitset for large delimiter sets rather than assuming that added setup cost is beneficial for typical small sets.

## Validation

Run the normal test suite:

```sh
make test
```

Build the example:

```sh
make example
```

Run AddressSanitizer + UndefinedBehaviorSanitizer validation:

```sh
make sanitize
```

The tests cover whitespace splitting, custom/repeated delimiters, empty input, delimiter-only input, an empty delimiter set, UTF-8 byte preservation, zero-allocation iteration, invalid arguments, ownership cleanup, and token-array termination.

## Design notes and future research

The first milestone prioritizes correctness and a clear contract over premature specialization. Useful next experiments include:

1. benchmark `strchr` delimiter lookup against a 256-entry lookup table across input sizes and delimiter-set cardinalities;
2. add randomized/property tests comparing iterator and materialized results;
3. fuzz arbitrary byte strings and delimiter sets under sanitizers;
4. measure copy-based vs zero-copy throughput, allocations, cache behavior, and branch behavior;
5. evaluate streaming/chunked tokenization for inputs that cannot be held contiguously in memory;
6. investigate optional quote/escape handling as a separate parser mode without changing the simple splitter semantics;
7. benchmark SIMD delimiter classification only if scalar measurements show a meaningful bottleneck.

Performance changes should be justified by reproducible measurements rather than intuition.

## Repository layout

```text
.
├── tokenizer.c
├── tokenizer.h
├── tests/
│   └── test_tokenizer.c
├── examples/
│   └── example.c
├── Makefile
├── LICENSE
└── README.md
```

## License

This project is licensed under the [MIT License](LICENSE). Copyright (c) 2026 Eddy Todd.
