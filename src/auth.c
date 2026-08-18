#include <stdio.h>
#include <string.h>
#include "auth.h"
 
#define MAX_ATTEMPTS 3
 
static const char *USERNAME = "admin";
static const char *PASSWORD = "admin123";
 
int authenticate(void)
{
    char user[50];
    char pass[50];
    int attempts = 0;
 
    while(attempts < MAX_ATTEMPTS)
    {
        printf("Username: ");
        scanf("%49s", user);
        printf("Password: ");
        scanf("%49s", pass);
 
        if(strcmp(user, USERNAME) == 0 && strcmp(pass, PASSWORD) == 0)
        {
            printf("\nAuthentication Successful\n");
            return 1;
        }
        else
        {
            attempts++;
            printf("Invalid credentials. Attempts left: %d\n\n", MAX_ATTEMPTS - attempts);
        }
    }
    printf("Authentication Failed. Access Denied.\n");
    return 0;
}
