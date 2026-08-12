#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
 
#define MAX_INPUT 256
#define MAX_ARGS 20
#define MAX_ATTEMPTS 3
 
const char *username = "admin";
const char *password = "admin123";
 
const char *allowed_commands[] = {
    "ls", "pwd", "date", "whoami", "echo", "cat", "clear", NULL
};
 
void log_attempt(const char *cmd, const char *status) {
    FILE *fp = fopen("logs/access.log", "a");
    if (fp == NULL) return;
    time_t now = time(NULL);
    char *t = ctime(&now);
    t[strlen(t) - 1] = '\0';
    fprintf(fp, "[%s] Command: \"%s\" - %s\n", t, cmd, status);
    fclose(fp);
}
 
int authenticate() {
    char user[50], pass[50];
    int attempts = 0;
 
    while (attempts < MAX_ATTEMPTS) {
        printf("Username: ");
        scanf("%49s", user);
        printf("Password: ");
        scanf("%49s", pass);
 
        if (strcmp(user, username) == 0 && strcmp(pass, password) == 0) {
            printf("\nAuthentication Successful\n");
            log_attempt(user, "LOGIN SUCCESS");
            return 1;
        } else {
            attempts++;
            log_attempt(user, "LOGIN FAILED");
            printf("Invalid credentials. Attempts left: %d\n\n", MAX_ATTEMPTS - attempts);
        }
    }
    printf("Authentication Failed. Access Denied.\n");
    return 0;
}
 
int is_allowed(const char *cmd) {
    for (int i = 0; allowed_commands[i] != NULL; i++) {
        if (strcmp(cmd, allowed_commands[i]) == 0)
            return 1;
    }
    return 0;
}
 
int has_dangerous_chars(const char *input) {
    const char *blocked = ";|&><`$\\";
    for (int i = 0; input[i] != '\0'; i++) {
        if (strchr(blocked, input[i]) != NULL)
            return 1;
    }
    if (strstr(input, "..") != NULL)
        return 1;
    return 0;
}
 
int main() {
    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];
    char *args[MAX_ARGS];
 
    printf("===== Secure Restricted Shell =====\n");
 
    if (!authenticate()) {
        exit(1);
    }
 
    while (1) {
        printf("\nrshell> ");
        if (fgets(input, MAX_INPUT, stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';
 
        if (strlen(input) == 0)
            continue;
 
        if (strcmp(input, "exit") == 0) {
            log_attempt(input, "SESSION ENDED");
            break;
        }
 
        if (has_dangerous_chars(input)) {
            printf("Blocked: unsafe characters detected\n");
            log_attempt(input, "BLOCKED - UNSAFE INPUT");
            continue;
        }
 
        strcpy(input_copy, input);
        int argc_local = 0;
        char *token = strtok(input_copy, " ");
        while (token != NULL && argc_local < MAX_ARGS - 1) {
            args[argc_local++] = token;
            token = strtok(NULL, " ");
        }
        args[argc_local] = NULL;
 
        if (argc_local == 0)
            continue;
 
        if (!is_allowed(args[0])) {
            printf("Command not permitted: %s\n", args[0]);
            log_attempt(input, "BLOCKED - NOT WHITELISTED");
            continue;
        }
 
        pid_t pid = fork();
        if (pid < 0) {
            printf("Fork failed\n");
        } else if (pid == 0) {
            execvp(args[0], args);
            printf("Execution failed\n");
            exit(1);
        } else {
            wait(NULL);
            log_attempt(input, "EXECUTED");
        }
    }
 
    printf("\nSession terminated.\n");
    return 0;
}
