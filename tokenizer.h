#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdlib.h>
#include <stdio.h>

// Checks if a given character is valid
char is_valid_char(char c);

// Counts the number of characters in the current lexeme
int word_len(char* str);

// Returns a copy of the provided char pointer
char* copy_word(char* str);

// Finds pointer to the start of the next token
char* token_start(char* str);

// Finds pointer to the end of the next token
char* token_end(char* str);

// Counts number of tokens in a string
int count_tokens(char* str); nt;

// Tokenizes a string into an array of tokens
char** tokenize(char* str);

// Prints all tokens
void print_tokens(char** tokens);

// Frees the memory allocated by the tokens
void free_tokens(char** tokens);

#endif
