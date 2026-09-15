#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "executor.h"

typedef struct {
    const char *username;
    const char *password;
    const char *role;
} UserRecord;

static const UserRecord USERS[] = {
    {"admin", "admin123", "administrator"},
    {"user", "user123", "standard_user"},
    {"guest", "guest123", "guest_user"}
};

static const int NUM_USERS = sizeof(USERS) / sizeof(USERS[0]);
static char active_user[50] = "anonymous";

const char *get_current_user(void)
{
    return active_user;
}

int authenticate(void)
{
    char user[50];
    char pass[50];
    int attempts = 0;

    printf("==========================================\n");
    printf("         RESTRICTED SHELL LOGIN           \n");
    printf("==========================================\n");

    while(attempts < MAX_ATTEMPTS)
    {
        printf("Username: ");
        if(scanf("%49s", user) != 1)
            return 0;
        printf("Password: ");
        if(scanf("%49s", pass) != 1)
            return 0;

        for(int i = 0; i < NUM_USERS; i++)
        {
            if(strcmp(user, USERS[i].username) == 0 && strcmp(pass, USERS[i].password) == 0)
            {
                strncpy(active_user, USERS[i].username, sizeof(active_user) - 1);
                active_user[sizeof(active_user) - 1] = '\0';
                printf("\n[+] Authentication Successful (Role: %s)\n\n", USERS[i].role);
                log_attempt(user, "LOGIN SUCCESS");
                
                int c;
                while((c = getchar()) != '\n' && c != EOF);
                return 1;
            }
        }

        attempts++;
        log_attempt(user, "LOGIN FAILED");
        printf("[-] Invalid credentials. Attempts left: %d\n\n", MAX_ATTEMPTS - attempts);
    }

    printf("[!] Authentication Failed. Access Denied.\n");
    return 0;
}
