/*
 * Practical Session 4: Process Synchronization (wait vs waitpid)
 * Forks multiple child processes and synchronizes using waitpid() in specific order.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("=== Practical 4: Synchronization with wait() and waitpid() ===\n");
    pid_t pids[3];

    for (int i = 0; i < 3; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            printf("[Child %d] PID: %d running (sleep %d sec)...\n", i + 1, getpid(), (3 - i));
            sleep(3 - i);
            printf("[Child %d] PID: %d done.\n", i + 1, getpid());
            exit(100 + i);
        }
    }

    printf("[Parent] Waiting specifically for Child 3 (PID %d) using waitpid()...\n", pids[2]);
    int status;
    waitpid(pids[2], &status, 0);
    printf("[Parent] Child 3 collected. Exit code: %d\n", WEXITSTATUS(status));

    printf("[Parent] Reaping remaining children using wait()...\n");
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        printf("[Parent] Reaped child PID %d with exit code: %d\n", wpid, WEXITSTATUS(status));
    }
    printf("[Parent] All children synchronized.\n");
    return 0;
}
