/*
 * Practical Session 5: Producer-Consumer Communication Using Anonymous Pipe
 * Parent produces data and Child consumes it via pipe().
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int pipefd[2];
    char buffer[BUFFER_SIZE];
    const char *message = "Hello from Parent Process via Anonymous Pipe!";

    printf("=== Practical 5: Producer-Consumer via Anonymous Pipe ===\n");

    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child Process: Consumer
        close(pipefd[1]); // Close unused write end
        printf("[Consumer Child] Waiting for data on pipe...\n");
        ssize_t bytes = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("[Consumer Child] Received Message: \"%s\" (%zd bytes)\n", buffer, bytes);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        // Parent Process: Producer
        close(pipefd[0]); // Close unused read end
        printf("[Producer Parent] Sending data to child on pipe...\n");
        write(pipefd[1], message, strlen(message));
        close(pipefd[1]); // Close write end to indicate EOF
        wait(NULL);
        printf("[Producer Parent] Child consumed data and exited cleanly.\n");
    }
    return 0;
}
