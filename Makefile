all:
	gcc src/restricted_shell.c -o bin/restricted_shell
run:
	./bin/restricted_shell
clean:
	rm -f bin/restricted_shell
	rm -f logs/access.log

