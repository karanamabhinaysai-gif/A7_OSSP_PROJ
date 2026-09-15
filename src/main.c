#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "auth.h"
#include "executor.h"
#include "builtin.h"
#include "signals.h"

int main()
{
    char input[MAX_INPUT];

    // Initialize signal handlers for SIGINT, SIGTSTP, SIGCHLD
    initialize_signals();

    printf("=======================================================\n");
    printf("     %s Framework — Version %s\n", SHELL_NAME, VERSION);
    printf("     Advanced Process Management & Security Control    \n");
    printf("=======================================================\n\n");

    // Authenticate user before shell access is granted
    if(!authenticate())
    {
        return EXIT_FAILURE;
    }

    printf("[+] Welcome, %s! Type 'help' for commands, 'exit' to quit.\n\n", get_current_user());

    while(1)
    {
        printf("myshell> ");
        fflush(stdout);

        if(fgets(input, sizeof(input), stdin) == NULL)
        {
            // EOF (Ctrl+D) reached
            printf("\n");
            break;
        }

        // Strip newline
        input[strcspn(input, "\r\n")] = '\0';

        if(strlen(input) == 0)
            continue;

        execute_command(input);
    }

    printf("\nGoodbye!\n");
    log_attempt("session", "SESSION CLOSED NORMALLY");
    return EXIT_SUCCESS;
}
