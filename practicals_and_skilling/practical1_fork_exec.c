/*
 * Practical Session 1: Command Execution using fork() and exec()
 * Accepts a Linux command as input, forks a child process, executes the command
 * using execvp(), waits in parent with wait(), and displays PIDs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    char command[256];
    char *args[10];
    int status;

    printf("=== Practical 1: Command Execution using fork() & exec() ===\n");
    printf("Enter a Linux command (e.g. ls, pwd, date): ");
    if (fgets(command, sizeof(command), stdin) == NULL) return 1;
    command[strcspn(command, "\r\n")] = '\0';

    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < 9) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL) return 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        printf("[Child Process] PID: %d, Parent PID: %d\n", getpid(), getppid());
        printf("[Child Process] Executing command: %s\n", args[0]);
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("[Parent Process] PID: %d, Created Child PID: %d\n", getpid(), pid);
        wait(&status);
        printf("[Parent Process] Child finished with exit status: %d\n", WEXITSTATUS(status));
    }
    return 0;
}
