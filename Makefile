CC       = clang
CFLAGS   = -Wall -Wextra -std=c23 -O2 -D_CRT_SECURE_NO_WARNINGS
SRC_DIR  = src
OBJ_DIR  = target
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET   = emu.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	$(TARGET) $(ARGS)

clean:
	@if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR)
	@if exist $(TARGET) del /Q $(TARGET)