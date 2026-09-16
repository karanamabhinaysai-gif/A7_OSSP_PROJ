CC = gcc
CFLAGS = -Wall -Wextra -g

PROGS = practical1 practical2 practical3 practical4 practical5

all: $(PROGS)

practical1: practical1_fork_exec.c
	$(CC) $(CFLAGS) practical1_fork_exec.c -o practical1

practical2: practical2_file_copy_syscalls.c
	$(CC) $(CFLAGS) practical2_file_copy_syscalls.c -o practical2

practical3: practical3_process_states.c
	$(CC) $(CFLAGS) practical3_process_states.c -o practical3

practical4: practical4_wait_waitpid.c
	$(CC) $(CFLAGS) practical4_wait_waitpid.c -o practical4

practical5: practical5_anonymous_pipe.c
	$(CC) $(CFLAGS) practical5_anonymous_pipe.c -o practical5

clean:
	rm -f $(PROGS) sample_source.txt sample_destination.txt
