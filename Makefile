CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -pthread
SRC = src/main.c \
      src/auth.c \
      src/executor.c \
      src/builtin.c \
      src/signals.c \
      src/pipes.c \
      src/redirect.c \
      src/thread.c
TARGET = bin/shellforge

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p bin logs
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

asan: $(SRC)
	@mkdir -p bin logs
	$(CC) $(CFLAGS) -fsanitize=address $(SRC) $(LDFLAGS) -o $(TARGET)
	@echo "Build successful with AddressSanitizer: $(TARGET)"

run: $(TARGET)
	@mkdir -p logs
	./$(TARGET)

valgrind: $(TARGET)
	@mkdir -p logs
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -rf bin/* logs/*.log
	@echo "Clean completed."
