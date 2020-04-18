#include "tokenizer.h"

#include <stdlib.h>
#include <stdio.h>

// Checks if a given character is valid
char is_valid_char(char c) {
    return c > 32 && c < 127;
}

// Counts the number of characters in the current lexeme
int word_len(char *str) {
    char *p = str;
    while (is_valid_char(*p++));
    return p - str - 1;
}

// Returns a copy of the provided char pointer
char* copy_word(char* str) {
    char* copy = (char*)malloc(sizeof(char) * (word_len(str) + 1));
    char* p = copy;
    while (is_valid_char(*str))
        *copy++ = *str++;
    *copy = '\0';
    return p;
}

// Finds pointer to the start of the next token
char *token_start(char *str) {
    while (!is_valid_char(*str)) {
        str++;
        if (!*str)
            return '\0';
    }
    return str;
}

// Finds pointer to the end of the next token
char *token_end(char *str) {
    while (is_valid_char(*str))
        str++;
    return str;
}

// Counts number of tokens in a string
int count_tokens(char *str) {
    int count = 0;
    while (*str) {
        if (is_valid_char(*str)) {
            count++;
            str = token_end(str);
        } else str++;
    }
    return count;
}


// Tokenizes a string into an array of tokens
char **tokenize(char *str) {
    int words = count_tokens(str);
    char **tokens = (char **) malloc(sizeof(char *) * (words + 1));
    char **t = tokens;
    while (words--) {
        str = token_start(str);
        *tokens++ = copy_word(str);
        str = token_end(str);
    }
    *tokens = '\0';
    return t;
}

// Prints all tokens
void print_tokens(char **tokens) {
    while (*tokens)
        printf("%s\n", *tokens++);
}

// Frees the memory allocated by the tokens
void free_tokens(char **tokens) {
    char **t = tokens;
    while (*tokens)
        free(*tokens++);
    free(t);
}