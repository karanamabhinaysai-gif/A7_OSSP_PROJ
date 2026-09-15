#ifndef EXECUTOR_H
#define EXECUTOR_H

void execute_command(char *line);
int is_allowed(const char *cmd);
int has_dangerous_chars(const char *input);
void log_attempt(const char *cmd, const char *status);

#endif
