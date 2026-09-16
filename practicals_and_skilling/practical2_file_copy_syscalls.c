/*
 * Practical Session 2: File Copy using Low-level System Calls
 * Uses open(), read(), write(), and close() system calls.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    printf("=== Practical 2: File Copy Using System Calls ===\n");

    const char *src_file = (argc > 1) ? argv[1] : "sample_source.txt";
    const char *dest_file = (argc > 2) ? argv[2] : "sample_destination.txt";

    // Create dummy source if not exists
    src_fd = open(src_file, O_WRONLY | O_CREAT, 0644);
    if (src_fd != -1) {
        write(src_fd, "OSSP File Copy Demo using open, read, write, close.\n", 52);
        close(src_fd);
    }

    src_fd = open(src_file, O_RDONLY);
    if (src_fd < 0) {
        perror("Error opening source file");
        return 1;
    }

    dest_fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("Error opening destination file");
        close(src_fd);
        return 1;
    }

    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            return 1;
        }
    }

    printf("Successfully copied contents from %s to %s.\n", src_file, dest_file);
    close(src_fd);
    close(dest_fd);
    return 0;
}
