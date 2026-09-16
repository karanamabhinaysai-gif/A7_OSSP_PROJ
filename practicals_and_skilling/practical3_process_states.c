/*
 * Practical Session 3: Process Lifecycle and States
 * Demonstrates fork(), PID/PPID inspection, and states during sleep & wait.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    printf("=== Practical 3: Process Lifecycle & States ===\n");
    printf("[Initial] Parent Process PID: %d, PPID: %d\n", getpid(), getppid());

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        printf("[Child Running] PID: %d, PPID: %d\n", getpid(), getppid());
        printf("[Child] Entering sleep for 2 seconds (Sleeping State)...\n");
        sleep(2);
        printf("[Child] Waking up and terminating (Exit)...\n");
        exit(42);
    } else {
        printf("[Parent Waiting] Waiting for Child PID %d...\n", pid);
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            printf("[Parent] Child exited normally with code: %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}
