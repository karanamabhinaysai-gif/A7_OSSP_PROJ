CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/main.c \
      src/auth.c \
      src/executor.c \
      src/builtin.c \
      src/signals.c
TARGET = bin/shellforge

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p bin logs
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

run: $(TARGET)
	@mkdir -p logs
	./$(TARGET)

clean:
	rm -rf bin/* logs/*.log
	@echo "Clean completed."
