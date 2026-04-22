CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS :=
TUI_LDFLAGS := -lncurses -lpthread

SRC := src/main.c src/scorer.c
BIN_DIR := bin
OBJ_DIR := build
TARGET := $(BIN_DIR)/prompt-trainer
TEST_TARGET := $(BIN_DIR)/test-scorer
TEST_LAYOUT_TARGET := $(BIN_DIR)/test-tui-layout
TEST_HISTORY_TARGET := $(BIN_DIR)/test-tui-history
TEST_INPUT_TARGET := $(BIN_DIR)/test-tui-input
TEST_ASYNC_TARGET := $(BIN_DIR)/test-tui-async
TEST_TEXTWRAP_TARGET := $(BIN_DIR)/test-tui-textwrap

.PHONY: all clean run test

all: $(TARGET)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/main.o: src/main.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/scorer.o: src/scorer.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_layout.o: src/tui/layout.c include/tui/layout.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_history.o: src/tui/history.c include/tui/history.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_input.o: src/tui/input.c include/tui/input.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_async.o: src/tui/async.c include/tui/async.h include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_app.o: src/tui/app.c include/tui/app.h include/tui/layout.h include/tui/history.h include/tui/input.h include/tui/async.h include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_render.o: src/tui/render.c include/tui/render.h include/tui/layout.h include/tui/history.h include/tui/input.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tui_textwrap.o: src/tui/textwrap.c include/tui/textwrap.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_scorer.o: tests/test_scorer.c include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_tui_layout.o: tests/test_tui_layout.c include/tui/layout.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_tui_history.o: tests/test_tui_history.c include/tui/history.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_tui_input.o: tests/test_tui_input.c include/tui/input.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_tui_async.o: tests/test_tui_async.c include/tui/async.h include/scorer.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_tui_textwrap.o: tests/test_tui_textwrap.c include/tui/textwrap.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_DIR)/main.o $(OBJ_DIR)/scorer.o $(OBJ_DIR)/tui_app.o $(OBJ_DIR)/tui_layout.o $(OBJ_DIR)/tui_history.o $(OBJ_DIR)/tui_input.o $(OBJ_DIR)/tui_async.o $(OBJ_DIR)/tui_render.o $(OBJ_DIR)/tui_textwrap.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) $(TUI_LDFLAGS) -o $@

$(TEST_TARGET): $(OBJ_DIR)/test_scorer.o $(OBJ_DIR)/scorer.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_LAYOUT_TARGET): $(OBJ_DIR)/test_tui_layout.o $(OBJ_DIR)/tui_layout.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_HISTORY_TARGET): $(OBJ_DIR)/test_tui_history.o $(OBJ_DIR)/tui_history.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_INPUT_TARGET): $(OBJ_DIR)/test_tui_input.o $(OBJ_DIR)/tui_input.o $(OBJ_DIR)/tui_textwrap.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_ASYNC_TARGET): $(OBJ_DIR)/test_tui_async.o $(OBJ_DIR)/tui_async.o $(OBJ_DIR)/scorer.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -lpthread -o $@

$(TEST_TEXTWRAP_TARGET): $(OBJ_DIR)/test_tui_textwrap.o $(OBJ_DIR)/tui_textwrap.o | $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET) $(TEST_TARGET) $(TEST_LAYOUT_TARGET) $(TEST_HISTORY_TARGET) $(TEST_INPUT_TARGET) $(TEST_ASYNC_TARGET) $(TEST_TEXTWRAP_TARGET)
	./$(TEST_TARGET)
	./$(TEST_LAYOUT_TARGET)
	./$(TEST_HISTORY_TARGET)
	./$(TEST_INPUT_TARGET)
	./$(TEST_ASYNC_TARGET)
	./$(TEST_TEXTWRAP_TARGET)
	sh tests/test_cli.sh

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
