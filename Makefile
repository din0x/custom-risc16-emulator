CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -O2 -D_CRT_SECURE_NO_WARNINGS

SRC_DIR := src
OBJ_DIR := target

TARGET  := $(OBJ_DIR)/risc16
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all build run clean

all: build

build: $(TARGET)

$(TARGET): $(OBJS) | $(OBJ_DIR)
	@echo LD $@
	@$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

run: build
	$(TARGET) $(ARGS)

ifeq ($(OS),Windows_NT)
clean:
	@if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR)
else
clean:
	@rm -rf $(OBJ_DIR)
endif
