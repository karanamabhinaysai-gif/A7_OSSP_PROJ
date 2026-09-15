#ifndef AUTH_H
#define AUTH_H

#define MAX_ATTEMPTS 3

int authenticate(void);
const char *get_current_user(void);

#endif
