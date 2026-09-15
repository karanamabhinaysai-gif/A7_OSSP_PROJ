# RestrictedShell — Secure Command Execution Framework

A secure, modular, and resilient Linux shell (`rshell`) developed in C for the **Operating Systems and Systems Programming (OSSP)** course. The framework provides user authentication, strict command whitelisting, dangerous character sanitization, built-in commands execution in the parent process, robust process lifecycle synchronization via `waitpid()`, asynchronous signal management (`SIGINT`, `SIGTSTP`, `SIGCHLD`), and tamper-evident audit logging.

---

## Architecture & System Design

```
+-------------------------------------------------------------+
|                      RestrictedShell                        |
+-------------------------------------------------------------+
                              |
                              v
                  [ 1. User Authentication ]
             (3 attempts limit, admin / user / guest)
                              |
                              v
                   [ 2. Interactive REPL ]
                (Signal handlers active: SIGINT)
                              |
                              v
                 [ 3. Input Sanitization ]
          Blocks: ; | & > < ` $ \ and ".." traversal
                              |
              +---------------+---------------+
              |                               |
              v                               v
    [ 4. Built-in Commands ]       [ 5. Whitelist Validator ]
    (cd, pwd, env, history,         Allowed: ls, pwd, date,
     clear, help, exit)              whoami, echo, cat, uname, df
              |                               |
              v                               v
    [ Executed in Parent ]         [ fork() + execvp() ]
              |                               |
              |                               v
              |                    [ waitpid() & Diagnostics ]
              |                    (WIFEXITED, WIFSIGNALED, perror)
              |                               |
              +---------------+---------------+
                              |
                              v
                   [ 6. Audit Trail Logging ]
                      (logs/access.log)
```

---

## Key Features

### 1. Multi-User Authentication Gate (`auth.c` / `auth.h`)
- Prevents unauthenticated access to the shell prompt.
- Enforces role-based authentication (`admin`, `user`, `guest`).
- Limits failed attempts to 3 before terminating the session.
- Records all login outcomes (`LOGIN SUCCESS`, `LOGIN FAILED`) to the audit trail.

### 2. Built-in Commands (`builtin.c` / `builtin.h`)
- Executed directly inside the parent process to modify the shell's state:
  - `cd <dir>`: Changes working directory using `chdir()`. Handles `~` (home directory), `..`, and relative paths.
  - `pwd`: Displays current working directory using `getcwd()`.
  - `env`: Inspects environment variables (`USER`, `HOME`, `PATH`, `SHELL`, `PWD`).
  - `history`: Displays the sequential log of commands executed in the current session.
  - `clear`: Clears terminal screen using ANSI escape sequences.
  - `help`: Interactive command reference guide.
  - `exit`: Cleanly terminates the session and closes open handles.

### 3. Command Whitelisting & Input Sanitization (`executor.c` / `executor.h`)
- **Strict Allow-List**: Only permitted system utilities (`ls`, `pwd`, `date`, `whoami`, `echo`, `cat`, `clear`, `uname`, `df`) can execute.
- **Injection Prevention**: Blocks shell metacharacters (`;`, `|`, `&`, `>`, `<`, ``` ` ```, `$`, `\`) to eliminate command chaining and subshell execution vulnerabilities.
- **Direct Execution**: Invokes `execvp()` directly rather than `system()`, preventing shell interpretation bugs.

### 4. Advanced Process Management (`executor.c`)
- Uses `fork()` to create an isolated child execution context.
- Synchronizes using `waitpid()` with `WUNTRACED` and handles `EINTR` signal interruptions.
- Inspects child exit status with `WIFEXITED(status)`, `WEXITSTATUS(status)`, and `WIFSIGNALED(status)`.
- Real system diagnostics through `perror("RestrictedShell")`.

### 5. Signal Safety & Zombie Process Prevention (`signals.c` / `signals.h`)
- Configured using standard POSIX `sigaction()`:
  - **`SIGINT` (Ctrl+C)**: Intercepted; displays an informative message to type `exit`, preventing accidental shell termination.
  - **`SIGTSTP` (Ctrl+Z)**: Ignored gracefully in foreground interactive mode.
  - **`SIGCHLD`**: Asynchronously reaps terminating child processes in the background using `waitpid(-1, NULL, WNOHANG)` to completely prevent zombie processes.

### 6. Audit Logging (`logs/access.log`)
- Every interaction is timestamped using `time()` and `ctime()`:
  - Login attempts (`LOGIN SUCCESS`, `LOGIN FAILED`)
  - Whitelist execution (`EXECUTED (Exit Code: 0)`)
  - Non-zero failure exits (`EXECUTED - NONZERO EXIT (code)`)
  - Signal terminations (`TERMINATED BY SIGNAL (sig)`)
  - Blocked commands (`BLOCKED - NOT WHITELISTED`, `BLOCKED - UNSAFE INPUT`)
  - Built-in executions and session terminations (`SESSION CLOSED NORMALLY`)

---

## Repository Structure

```
RestrictedShell/
├── Makefile
├── README.md
├── .gitignore
├── include/
│   ├── shell.h       # Core definitions, version constants
│   ├── auth.h        # Authentication prototypes and roles
│   ├── builtin.h     # Built-in command dispatcher and history
│   ├── executor.h    # Sanitization, whitelisting, and execution
│   └── signals.h     # POSIX signal handler definitions
├── src/
│   ├── main.c        # Shell entry point and REPL loop
│   ├── auth.c        # Credential verification & rate limiter
│   ├── builtin.c     # Built-ins (cd, pwd, env, history, etc.)
│   ├── executor.c    # Whitelist validator, fork/execvp/waitpid
│   └── signals.c     # Signal handlers (SIGINT, SIGCHLD, SIGTSTP)
├── logs/
│   └── access.log    # Tamper-evident audit log
└── bin/
    └── shellforge    # Compiled executable binary
```

---

## Build and Run

### Prerequisites
- GCC Compiler (`gcc`)
- Make (`make`)
- Linux / Ubuntu / WSL2 / POSIX-compliant environment

### Compilation
```bash
make clean
make
```

### Execution
```bash
make run
```

---

## Sample Interactive Session

```text
=======================================================
     RestrictedShell Framework — Version 6.0
     Advanced Process Management & Security Control    
=======================================================

==========================================
         RESTRICTED SHELL LOGIN           
==========================================
Username: admin
Password: admin123

[+] Authentication Successful (Role: administrator)

[+] Welcome, admin! Type 'help' for commands, 'exit' to quit.

myshell> pwd
/home/student/RestrictedShell

myshell> cd src
Directory changed to: /home/student/RestrictedShell/src

myshell> pwd
/home/student/RestrictedShell/src

myshell> env
--- Environment Variables ---
USER   = student
HOME   = /home/student
PATH   = /usr/local/sbin:/usr/local/bin:/usr/bin
SHELL  = /bin/bash
PWD    = /home/student/RestrictedShell/src
-----------------------------

myshell> whoami
student

myshell> rm -rf /
[-] Command not permitted: rm (Type 'help' for allowed commands)

myshell> ls; whoami
[-] Blocked: unsafe metacharacters detected (; | & > < ` $ \)

myshell> history
--- Command History ---
   1  pwd
   2  cd src
   3  pwd
   4  env
   5  whoami
   6  history
-----------------------

myshell> exit
Terminating RestrictedShell session.
```

---

## Audit Log Example (`logs/access.log`)

```text
[Tue Sep 15 19:45:01 2026] Command: "admin" - LOGIN SUCCESS
[Tue Sep 15 19:45:10 2026] Command: "pwd" - BUILTIN EXECUTED
[Tue Sep 15 19:45:15 2026] Command: "cd src" - BUILTIN EXECUTED - cd
[Tue Sep 15 19:45:20 2026] Command: "env" - BUILTIN EXECUTED
[Tue Sep 15 19:45:25 2026] Command: "whoami" - EXECUTED (Exit Code: 0)
[Tue Sep 15 19:45:30 2026] Command: "rm -rf /" - BLOCKED - NOT WHITELISTED
[Tue Sep 15 19:45:35 2026] Command: "ls; whoami" - BLOCKED - UNSAFE INPUT
[Tue Sep 15 19:45:40 2026] Command: "exit" - SESSION ENDED
```

---

## Project Timeline & Weekly Milestones

| Week | Phase / Milestone | Status | Key Deliverables |
| :--- | :--- | :---: | :--- |
| **Week 1** | REPL Foundations | Completed | Basic read-eval-print loop and repository initialization |
| **Week 2** | Dynamic Memory Management | Completed | Arbitrary length input buffer handling |
| **Week 3** | Authentication & Whitelisting | Completed | User login gate, command whitelist, metacharacter blocking, audit log |
| **Week 4** | Process Control & Diagnostics | Completed | `fork()`, `execvp()`, `waitpid()` loop with `WIFEXITED`/`WIFSIGNALED`, `perror()` |
| **Week 5** | Built-in Commands & Environment | Completed | `cd`, `pwd`, `help`, `clear`, `env`, `history`, `chdir()`, `getcwd()`, `getenv()` |
| **Week 6** | Signals & Process Management | Completed | POSIX `sigaction()` handlers for `SIGINT` (Ctrl+C survival) and `SIGCHLD` (async zombie cleanup) |

---

## Coursework & Team Information
- **Course**: Operating Systems and Systems Programming (OSSP) — 25CS2104E
- **Project Title**: Secure Command Execution Framework — Restricted Shell with User Authentication
- **Team Members**:
  - Abhinay Sai (2510030103)
  - K V Srinath (2510030106)
  - Poli Naidu (2510030160)
  - Chandu (2510030083)
