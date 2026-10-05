#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "redirect.h"
#include "executor.h"

int execute_redirection(char **args)
{
    int i;
    for (i = 0; args[i] != NULL; i++)
    {
        /* 1. Append output redirection: >> */
        if (strcmp(args[i], ">>") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "[-] Syntax error: Missing target file after '>>'\n");
                return 1;
            }

            char *file = args[i + 1];
            args[i] = NULL; // Terminate argument array before redirection operator

            if (!is_allowed(args[0]))
            {
                printf("[-] Command not permitted: %s (Type 'help' for allowed commands)\n", args[0]);
                log_attempt(args[0], "BLOCKED - REDIRECTION NOT WHITELISTED");
                return 1;
            }

            int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0)
            {
                perror("RestrictedShell: open (>>)");
                return 1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("RestrictedShell: fork failed for redirection");
                close(fd);
                return 1;
            }

            if (pid == 0)
            {
                dup2(fd, STDOUT_FILENO);
                close(fd);
                execvp(args[0], args);
                perror("RestrictedShell: execvp failed");
                exit(EXIT_FAILURE);
            }

            close(fd);
            int status;
            waitpid(pid, &status, 0);
            log_attempt(file, "EXECUTED REDIRECTION (>>)");
            return 1;
        }

        /* 2. Error redirection: 2> */
        if (strcmp(args[i], "2>") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "[-] Syntax error: Missing target file after '2>'\n");
                return 1;
            }

            char *file = args[i + 1];
            args[i] = NULL;

            if (!is_allowed(args[0]))
            {
                printf("[-] Command not permitted: %s (Type 'help' for allowed commands)\n", args[0]);
                log_attempt(args[0], "BLOCKED - REDIRECTION NOT WHITELISTED");
                return 1;
            }

            int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("RestrictedShell: open (2>)");
                return 1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("RestrictedShell: fork failed for redirection");
                close(fd);
                return 1;
            }

            if (pid == 0)
            {
                dup2(fd, STDERR_FILENO);
                close(fd);
                execvp(args[0], args);
                perror("RestrictedShell: execvp failed");
                exit(EXIT_FAILURE);
            }

            close(fd);
            int status;
            waitpid(pid, &status, 0);
            log_attempt(file, "EXECUTED REDIRECTION (2>)");
            return 1;
        }

        /* 3. Standard output redirection: > */
        if (strcmp(args[i], ">") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "[-] Syntax error: Missing target file after '>'\n");
                return 1;
            }

            char *file = args[i + 1];
            args[i] = NULL;

            if (!is_allowed(args[0]))
            {
                printf("[-] Command not permitted: %s (Type 'help' for allowed commands)\n", args[0]);
                log_attempt(args[0], "BLOCKED - REDIRECTION NOT WHITELISTED");
                return 1;
            }

            int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("RestrictedShell: open (>)");
                return 1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("RestrictedShell: fork failed for redirection");
                close(fd);
                return 1;
            }

            if (pid == 0)
            {
                dup2(fd, STDOUT_FILENO);
                close(fd);
                execvp(args[0], args);
                perror("RestrictedShell: execvp failed");
                exit(EXIT_FAILURE);
            }

            close(fd);
            int status;
            waitpid(pid, &status, 0);
            log_attempt(file, "EXECUTED REDIRECTION (>)");
            return 1;
        }

        /* 4. Input redirection: < */
        if (strcmp(args[i], "<") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "[-] Syntax error: Missing source file after '<'\n");
                return 1;
            }

            char *file = args[i + 1];
            args[i] = NULL;

            if (!is_allowed(args[0]))
            {
                printf("[-] Command not permitted: %s (Type 'help' for allowed commands)\n", args[0]);
                log_attempt(args[0], "BLOCKED - REDIRECTION NOT WHITELISTED");
                return 1;
            }

            int fd = open(file, O_RDONLY);
            if (fd < 0)
            {
                perror("RestrictedShell: open (<)");
                return 1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("RestrictedShell: fork failed for redirection");
                close(fd);
                return 1;
            }

            if (pid == 0)
            {
                dup2(fd, STDIN_FILENO);
                close(fd);
                execvp(args[0], args);
                perror("RestrictedShell: execvp failed");
                exit(EXIT_FAILURE);
            }

            close(fd);
            int status;
            waitpid(pid, &status, 0);
            log_attempt(file, "EXECUTED REDIRECTION (<)");
            return 1;
        }
    }
    return 0;
}
