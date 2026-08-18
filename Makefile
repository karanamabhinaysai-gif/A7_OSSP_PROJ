CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/main.c \
      src/auth.c \
      src/executor.c
TARGET = bin/shellforge
 
all: $(TARGET)
 
$(TARGET):
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
 
run:
	./$(TARGET)
 
clean:
	rm -rf bin/*

