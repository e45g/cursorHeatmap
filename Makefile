# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude -O3
# Source directory
SRC_DIR = src
LIB_DIR = lib
# Output directory
OUT_DIR = bin
# Output executable
OUTPUT = cursor_heatmap.exe

# Get all .c files in the source directory and lib directory
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)

# Rule to build the executable
$(OUTPUT): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

# Phony target to clean up generated files
.PHONY: clean
clean:
	rm -f $(OUT_DIR)/$(OUTPUT)
