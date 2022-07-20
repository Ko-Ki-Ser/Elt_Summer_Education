all: fork_and_pipe.out fork_exec_and_mkfifo_parent.out fork_exec_and_mkfifo_child.out

fork_and_pipe.out: fork_and_pipe.c
	gcc -o fork_and_pipe.out fork_and_pipe.c

fork_exec_and_mkfifo_parent.out: fork_exec_and_mkfifo_parent.c
	gcc -o fork_exec_and_mkfifo_parent.out fork_exec_and_mkfifo_parent.c -g

fork_exec_and_mkfifo_child.out: fork_exec_and_mkfifo_child.c
	gcc -o fork_exec_and_mkfifo_child.out fork_exec_and_mkfifo_child.c -g

clean:
	-rm fork_and_pipe.out fork_exec_and_mkfifo_parent.out fork_exec_and_mkfifo_child.out