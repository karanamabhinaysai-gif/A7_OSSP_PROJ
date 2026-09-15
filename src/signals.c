#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "signals.h"
#include "executor.h"

static void sigint_handler(int sig)
{
    (void)sig;
    printf("\n[!] ShellForge: Caught SIGINT (Ctrl+C). Type 'exit' to quit.\n");
    printf("myshell> ");
    fflush(stdout);
    log_attempt("SIGINT", "SIGNAL CAUGHT - Ctrl+C");
}

static void sigtstp_handler(int sig)
{
    (void)sig;
    printf("\n[!] ShellForge: Caught SIGTSTP (Ctrl+Z). Background stop ignored.\n");
    printf("myshell> ");
    fflush(stdout);
    log_attempt("SIGTSTP", "SIGNAL CAUGHT - Ctrl+Z");
}

static void sigchld_handler(int sig)
{
    (void)sig;
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0)
    {
        // Child reaped asynchronously to prevent zombies
    }
}

void initialize_signals(void)
{
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}
