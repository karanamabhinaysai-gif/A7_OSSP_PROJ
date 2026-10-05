#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "executor.h"
#include "builtin.h"
#include "shell.h"
#include "pipes.h"
#include "redirect.h"

static const char *ALLOWED_COMMANDS[] =
{
    "ls", "pwd", "date", "whoami", "echo", "cat", "clear", "uname", "df",
    "wc", "grep", "ps", "sort", "head", "tail", NULL
};

int is_allowed(const char *cmd)
{
    if(cmd == NULL)
        return 0;
    for(int i = 0; ALLOWED_COMMANDS[i] != NULL; i++)
    {
        if(strcmp(cmd, ALLOWED_COMMANDS[i]) == 0)
            return 1;
    }
    return 0;
}

int has_dangerous_chars(const char *input)
{
    const char *blocked = ";&`$\\";
    for(int i = 0; input[i] != '\0'; i++)
    {
        if(strchr(blocked, input[i]) != NULL)
            return 1;
    }
    return 0;
}

void log_attempt(const char *cmd, const char *status)
{
    FILE *fp = fopen("logs/access.log", "a");
    if(fp == NULL)
        return;
    time_t now = time(NULL);
    char *t = ctime(&now);
    if(t != NULL)
        t[strlen(t) - 1] = '\0';
    else
        t = "N/A";
    fprintf(fp, "[%s] Command: \"%s\" - %s\n", t, cmd, status);
    fclose(fp);
}

void execute_command(char *line)
{
    char copy[MAX_INPUT];
    char *args[MAX_ARGS];
    int argc_local = 0;
    pid_t pid;
    int status;

    if(line == NULL || strlen(line) == 0)
        return;

    // 1. Security Check: Dangerous metacharacters
    if(has_dangerous_chars(line))
    {
        printf("[-] Blocked: unsafe metacharacters detected (; & ` $ \\)\n");
        log_attempt(line, "BLOCKED - UNSAFE INPUT");
        return;
    }

    // 2. Pipeline Execution: Check for Pipe (|)
    if(strchr(line, '|') != NULL)
    {
        char pipe_copy[MAX_INPUT];
        strncpy(pipe_copy, line, sizeof(pipe_copy) - 1);
        pipe_copy[sizeof(pipe_copy) - 1] = '\0';

        char *left = strtok(pipe_copy, "|");
        char *right = strtok(NULL, "|");

        if(left == NULL || right == NULL)
        {
            printf("[-] Invalid pipe command syntax\n");
            log_attempt(line, "BLOCKED - INVALID PIPE SYNTAX");
            return;
        }

        char *cmd1_args[MAX_ARGS];
        char *cmd2_args[MAX_ARGS];
        int c1 = 0, c2 = 0;

        char *tok = strtok(left, " \t");
        while(tok != NULL && c1 < MAX_ARGS - 1)
        {
            cmd1_args[c1++] = tok;
            tok = strtok(NULL, " \t");
        }
        cmd1_args[c1] = NULL;

        tok = strtok(right, " \t");
        while(tok != NULL && c2 < MAX_ARGS - 1)
        {
            cmd2_args[c2++] = tok;
            tok = strtok(NULL, " \t");
        }
        cmd2_args[c2] = NULL;

        if(c1 == 0 || c2 == 0)
        {
            printf("[-] Missing command in pipeline\n");
            log_attempt(line, "BLOCKED - INCOMPLETE PIPE");
            return;
        }

        if(!is_allowed(cmd1_args[0]) || !is_allowed(cmd2_args[0]))
        {
            printf("[-] Pipeline command not permitted (Type 'help' for allowed commands)\n");
            log_attempt(line, "BLOCKED - PIPE COMMAND NOT WHITELISTED");
            return;
        }

        add_history(line);
        execute_pipe(cmd1_args, cmd2_args);
        log_attempt(line, "EXECUTED PIPELINE");
        return;
    }

    // 3. Tokenize standard command into argument array
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *token = strtok(copy, " \t");
    while(token != NULL && argc_local < MAX_ARGS - 1)
    {
        args[argc_local++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc_local] = NULL;

    if(argc_local == 0)
        return;

    // Record in history
    add_history(line);

    // 4. Check and execute built-in commands in parent process
    if(is_builtin(args[0]))
    {
        execute_builtin(args);
        return;
    }

    // 5. I/O Redirection Execution: Check for >, >>, <, 2>
    if(execute_redirection(args))
    {
        return;
    }

    // 6. Whitelist check for external commands
    if(!is_allowed(args[0]))
    {
        printf("[-] Command not permitted: %s (Type 'help' for allowed commands)\n", args[0]);
        log_attempt(line, "BLOCKED - NOT WHITELISTED");
        return;
    }

    // 7. Fork and Execute external process
    pid = fork();

    if(pid < 0)
    {
        perror("RestrictedShell: fork failed");
        log_attempt(line, "FORK FAILED");
        return;
    }
    else if(pid == 0)
    {
        // Child process
        if(execvp(args[0], args) == -1)
        {
            perror("RestrictedShell: execution failed");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process waits for foreground child
        do
        {
            if(waitpid(pid, &status, WUNTRACED) == -1)
            {
                if(errno == EINTR)
                    continue;
                break;
            }
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));

        if(WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if(exit_code == 0)
                log_attempt(line, "EXECUTED (Exit Code: 0)");
            else
            {
                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "EXECUTED - NONZERO EXIT (%d)", exit_code);
                log_attempt(line, log_msg);
            }
        }
        else if(WIFSIGNALED(status))
        {
            int sig_num = WTERMSIG(status);
            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "TERMINATED BY SIGNAL (%d)", sig_num);
            log_attempt(line, log_msg);
        }
    }
}
