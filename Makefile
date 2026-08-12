CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_tokenizer
EXAMPLE_BIN := $(BUILD_DIR)/tokenizer_example
SANITIZER_BIN := $(BUILD_DIR)/test_tokenizer_sanitize

.PHONY: all test example run-example sanitize clean

all: test example

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): tokenizer.c tokenizer.h tests/test_tokenizer.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. tokenizer.c tests/test_tokenizer.c -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(EXAMPLE_BIN): tokenizer.c tokenizer.h examples/example.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. tokenizer.c examples/example.c -o $@

example: $(EXAMPLE_BIN)

run-example: $(EXAMPLE_BIN)
	./$(EXAMPLE_BIN)

sanitize: | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -std=c11 -O1 -g -Wall -Wextra -Wpedantic \
		-fsanitize=address,undefined -fno-omit-frame-pointer -I. \
		tokenizer.c tests/test_tokenizer.c -o $(SANITIZER_BIN)
	./$(SANITIZER_BIN)

clean:
	rm -rf $(BUILD_DIR)
