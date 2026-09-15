#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"
#include "executor.h"

#define MAX_HISTORY 100

static char history_buffer[MAX_HISTORY][1024];
static int history_count = 0;

void add_history(const char *cmd)
{
    if(cmd == NULL || strlen(cmd) == 0)
        return;
    if(history_count < MAX_HISTORY)
    {
        strncpy(history_buffer[history_count], cmd, sizeof(history_buffer[history_count]) - 1);
        history_buffer[history_count][sizeof(history_buffer[history_count]) - 1] = '\0';
        history_count++;
    }
    else
    {
        for(int i = 0; i < MAX_HISTORY - 1; i++)
        {
            strcpy(history_buffer[i], history_buffer[i + 1]);
        }
        strncpy(history_buffer[MAX_HISTORY - 1], cmd, sizeof(history_buffer[MAX_HISTORY - 1]) - 1);
        history_buffer[MAX_HISTORY - 1][sizeof(history_buffer[MAX_HISTORY - 1]) - 1] = '\0';
    }
}

void print_history(void)
{
    printf("\n--- Command History ---\n");
    if(history_count == 0)
    {
        printf("No commands executed yet.\n");
        return;
    }
    for(int i = 0; i < history_count; i++)
    {
        printf(" %3d  %s\n", i + 1, history_buffer[i]);
    }
    printf("-----------------------\n\n");
}

int is_builtin(const char *cmd)
{
    if(cmd == NULL)
        return 0;
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "pwd") == 0 ||
            strcmp(cmd, "help") == 0 ||
            strcmp(cmd, "clear") == 0 ||
            strcmp(cmd, "env") == 0 ||
            strcmp(cmd, "history") == 0 ||
            strcmp(cmd, "exit") == 0);
}

int execute_builtin(char **args)
{
    char cwd[1024];
    if(args[0] == NULL)
        return 1;

    /* exit */
    if(strcmp(args[0], "exit") == 0)
    {
        printf("Terminating RestrictedShell session.\n");
        log_attempt("exit", "SESSION ENDED");
        exit(EXIT_SUCCESS);
    }

    /* pwd */
    if(strcmp(args[0], "pwd") == 0)
    {
        if(getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            log_attempt("pwd", "BUILTIN EXECUTED");
        }
        else
        {
            perror("pwd");
        }
        return 1;
    }

    /* cd */
    if(strcmp(args[0], "cd") == 0)
    {
        const char *target = args[1];
        if(target == NULL || strcmp(target, "~") == 0)
        {
            target = getenv("HOME");
            if(target == NULL)
                target = "/";
        }
        if(chdir(target) != 0)
        {
            perror("cd");
            log_attempt(args[0], "BUILTIN FAILED - cd");
        }
        else
        {
            if(getcwd(cwd, sizeof(cwd)) != NULL)
                printf("Directory changed to: %s\n", cwd);
            log_attempt(args[0], "BUILTIN EXECUTED - cd");
        }
        return 1;
    }

    /* clear */
    if(strcmp(args[0], "clear") == 0)
    {
        printf("\033[H\033[J");
        fflush(stdout);
        log_attempt("clear", "BUILTIN EXECUTED");
        return 1;
    }

    /* env */
    if(strcmp(args[0], "env") == 0)
    {
        printf("\n--- Environment Variables ---\n");
        printf("USER   = %s\n", getenv("USER") ? getenv("USER") : "N/A");
        printf("HOME   = %s\n", getenv("HOME") ? getenv("HOME") : "N/A");
        printf("PATH   = %s\n", getenv("PATH") ? getenv("PATH") : "N/A");
        printf("SHELL  = %s\n", getenv("SHELL") ? getenv("SHELL") : "N/A");
        if(getcwd(cwd, sizeof(cwd)) != NULL)
            printf("PWD    = %s\n", cwd);
        printf("-----------------------------\n\n");
        log_attempt("env", "BUILTIN EXECUTED");
        return 1;
    }

    /* history */
    if(strcmp(args[0], "history") == 0)
    {
        print_history();
        log_attempt("history", "BUILTIN EXECUTED");
        return 1;
    }

    /* help */
    if(strcmp(args[0], "help") == 0)
    {
        printf("\n=======================================================\n");
        printf("              RestrictedShell Help Guide               \n");
        printf("=======================================================\n");
        printf(" Built-in Commands (Executed in Parent Process):\n");
        printf("   cd <dir>     : Change working directory (supports ~ and ..)\n");
        printf("   pwd          : Print current working directory\n");
        printf("   env          : Display key environment variables\n");
        printf("   history      : Show command history for this session\n");
        printf("   clear        : Clear terminal screen\n");
        printf("   help         : Display this help message\n");
        printf("   exit         : Exit the restricted shell\n\n");
        printf(" Whitelisted External Commands (Executed via fork/exec):\n");
        printf("   ls, date, whoami, echo, cat, uname, df\n\n");
        printf(" Security Rules:\n");
        printf("   - Dangerous characters (; | & > < ` $ \\) are strictly blocked.\n");
        printf("   - All actions and attempts are logged in logs/access.log.\n");
        printf("=======================================================\n\n");
        log_attempt("help", "BUILTIN EXECUTED");
        return 1;
    }

    return 0;
}
