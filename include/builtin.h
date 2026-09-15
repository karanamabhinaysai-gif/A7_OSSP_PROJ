#ifndef BUILTIN_H
#define BUILTIN_H

int is_builtin(const char *cmd);
int execute_builtin(char **args);
void add_history(const char *cmd);
void print_history(void);

#endif
