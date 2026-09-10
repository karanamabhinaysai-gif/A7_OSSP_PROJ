# RestrictedShell

A secure command execution framework built in C — a restricted shell (`rshell`) that
authenticates a user before granting access, and once logged in, permits execution of
only a predefined, whitelisted set of commands. Built as a week-by-week
project-based learning series for an Operating Systems and Systems Programming (OSSP)
course.

## Overview

Unlike a normal shell, RestrictedShell does not allow arbitrary commands, shell
metacharacters, or directory traversal. Every command a user types is checked against
a whitelist and scanned for unsafe characters before it is ever executed, and every
attempt — successful or blocked — is written to an audit log.

## Features

### Authentication
- Username/password login gate before the shell prompt is reached
- Login attempts capped at 3 tries, after which access is denied
- Every login attempt (success or failure) is recorded in the audit log

### Restricted Command Execution
- Fixed whitelist of allowed commands: `ls`, `pwd`, `date`, `whoami`, `echo`, `cat`, `clear`
- Any command not on the whitelist is rejected before it can run
- Shell metacharacters (`; | & > < \` $ \`) and `..` sequences are blocked to prevent
  command injection and directory traversal
- Commands are never handed to a shell interpreter — arguments are parsed manually
  and executed directly with `execvp()`

### Process Control
- Each whitelisted command runs in its own child process created with `fork()`
- The parent synchronizes with `waitpid()` and inspects the exit status using
  `WIFEXITED()` / `WIFSIGNALED()` / `WEXITSTATUS()`, instead of a blind `wait(NULL)`
- Fork and exec failures are reported with `perror()`, showing the real system error
  instead of a generic message

### Audit Logging
- Every login attempt, executed command, and blocked command is timestamped and
  appended to `logs/access.log`
- The log distinguishes a clean exit (`EXECUTED`) from a command that exited with a
  nonzero status (`EXECUTED - NONZERO EXIT`)

## Repository Structure

```
RestrictedShell/
│
├── Makefile
├── README.md
├── .gitignore
│
├── include/
│   ├── shell.h
│   ├── auth.h
│   └── executor.h
│
├── src/
│   ├── main.c
│   ├── auth.c
│   └── executor.c
│
├── logs/
│   └── access.log
│
└── bin/
    └── shellforge
```

## Build and Run

```bash
make clean
make
make run
```

## Usage

```
=================================
RestrictedShell Version 1.0
=================================
Username: admin
Password: admin123

Authentication Successful
myshell> pwd
/home/user/RestrictedShell
myshell> whoami
user
myshell> rm -rf /
Command not permitted: rm
myshell> ls; whoami
Blocked: unsafe characters detected
myshell> exit
```

Default credentials (for testing): `admin` / `admin123`

## Viewing the Audit Log

```bash
cat logs/access.log
```

## Development Timeline

| Week | Milestone |
| --- | --- |
| Week 1–2 | Basic REPL shell loop for reading and echoing user input |
| Week 3 | Authentication, command whitelisting, and unsafe-character blocking; commands executed via `fork()` + `execvp()` |
| Week 4 | Upgraded process control: `waitpid()` with exit-status checking, `perror()`-based diagnostics, and status-aware logging |

## License

Educational project — built for coursework purposes.
