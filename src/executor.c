#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "executor.h"
 
#define MAX_ARGS 20
 
static const char *ALLOWED_COMMANDS[] =
{
    "ls", "pwd", "date", "whoami", "echo", "cat", "clear", NULL
};
 
static int is_allowed(const char *cmd)
{
    for(int i = 0; ALLOWED_COMMANDS[i] != NULL; i++)
    {
        if(strcmp(cmd, ALLOWED_COMMANDS[i]) == 0)
            return 1;
    }
    return 0;
}
 
static int has_dangerous_chars(const char *input)
{
    const char *blocked = ";|&><`$\\";
    for(int i = 0; input[i] != '\0'; i++)
    {
        if(strchr(blocked, input[i]) != NULL)
            return 1;
    }
    if(strstr(input, "..") != NULL)
        return 1;
    return 0;
}
 
static void log_attempt(const char *cmd, const char *status)
{
    FILE *fp = fopen("logs/access.log", "a");
    if(fp == NULL)
        return;
    time_t now = time(NULL);
    char *t = ctime(&now);
    t[strlen(t) - 1] = '\0';
    fprintf(fp, "[%s] Command: \"%s\" - %s\n", t, cmd, status);
    fclose(fp);
}
 
void execute_command(char *line)
{
    char copy[1024];
    char *args[MAX_ARGS];
    int argc_local = 0;
 
    if(has_dangerous_chars(line))
    {
        printf("Blocked: unsafe characters detected\n");
        log_attempt(line, "BLOCKED - UNSAFE INPUT");
        return;
    }
 
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
 
    char *token = strtok(copy, " ");
    while(token != NULL && argc_local < MAX_ARGS - 1)
    {
        args[argc_local++] = token;
        token = strtok(NULL, " ");
    }
    args[argc_local] = NULL;
 
    if(argc_local == 0)
        return;
 
    if(!is_allowed(args[0]))
    {
        printf("Command not permitted: %s\n", args[0]);
        log_attempt(line, "BLOCKED - NOT WHITELISTED");
        return;
    }
 
    pid_t pid = fork();
    if(pid < 0)
    {
        printf("Fork failed\n");
    }
    else if(pid == 0)
    {
        execvp(args[0], args);
        printf("Execution failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        wait(NULL);
        log_attempt(line, "EXECUTED");
    }
}

