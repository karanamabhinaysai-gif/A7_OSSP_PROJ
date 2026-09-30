#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pipes.h"

void execute_pipe(char **cmd1, char **cmd2)
{
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("RestrictedShell: pipe creation failed");
        return;
    }

    /* First child: execute cmd1 (writes output to pipe) */
    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        perror("RestrictedShell: fork failed for stage 1");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid1 == 0)
    {
        /* Child 1 closes unused read end */
        close(pipefd[0]);

        /* Redirect standard output to pipe write end */
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("RestrictedShell: dup2 failed on stage 1 stdout");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        execvp(cmd1[0], cmd1);
        perror("RestrictedShell: command 1 execution failed");
        exit(EXIT_FAILURE);
    }

    /* Second child: execute cmd2 (reads input from pipe) */
    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        perror("RestrictedShell: fork failed for stage 2");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, NULL, 0);
        return;
    }

    if (pid2 == 0)
    {
        /* Child 2 closes unused write end */
        close(pipefd[1]);

        /* Redirect standard input from pipe read end */
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("RestrictedShell: dup2 failed on stage 2 stdin");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        close(pipefd[0]);

        execvp(cmd2[0], cmd2);
        perror("RestrictedShell: command 2 execution failed");
        exit(EXIT_FAILURE);
    }

    /* Parent closes both pipe file descriptors so EOF is properly detected */
    close(pipefd[0]);
    close(pipefd[1]);

    /* Wait for both pipeline children to terminate */
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
