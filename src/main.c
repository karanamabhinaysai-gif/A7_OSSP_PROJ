#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/shell.h"
#include "../include/auth.h"
#include "../include/executor.h"
 
#define MAX_INPUT 1024
 
int main()
{
    char input[MAX_INPUT];
 
    printf("=================================\n");
    printf("%s Version %s\n", SHELL_NAME, VERSION);
    printf("=================================\n");
 
    if(!authenticate())
    {
        return EXIT_FAILURE;
    }
 
    while(1)
    {
        printf("myshell> ");
        if(fgets(input, MAX_INPUT, stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';
 
        if(strcmp(input, "exit") == 0)
        {
            break;
        }
 
        if(strlen(input) != 0)
            execute_command(input);
    }
    printf("Goodbye!\n");
    return 0;
}
