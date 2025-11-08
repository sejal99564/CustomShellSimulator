# ==== Custom Shell Simulator Makefile ====

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -O2
SRC = src/main.c
OBJ = $(SRC:src/%.c=build/%.o)
BIN = bin/mini_shell

# Default target
all: $(BIN)

# Link the final executable
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each .c file into build/
build/%.o: src/%.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Run the shell
run: $(BIN)
	./$(BIN)

# Clean up
clean:
	del /Q build\*.o 2>nul
	del /Q $(BIN) 2>nul
