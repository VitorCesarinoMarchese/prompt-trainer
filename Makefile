CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS :=

SRC := src/main.c src/scorer.c
BIN_DIR := bin
OBJ_DIR := build
TARGET := $(BIN_DIR)/prompt-trainer
TEST_TARGET := $(BIN_DIR)/test-scorer

.PHONY: all clean run test

all: $(TARGET)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/main.o: src/main.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/scorer.o: src/scorer.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_scorer.o: tests/test_scorer.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_DIR)/main.o $(OBJ_DIR)/scorer.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_TARGET): $(OBJ_DIR)/test_scorer.o $(OBJ_DIR)/scorer.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	sh tests/test_cli.sh

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
