#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

// передача номера сигнала и pid процесса 
// через аргументы командной строки
// SIGUSR1 - 10

int main (int argc, char** argv) {

	int sig = atoi(argv[2]);
	pid_t pid = atoi(argv[1]);

	for (int i = 0; i < 10; i++) {
		kill(pid, sig);
		sleep(1);
	}

	exit(EXIT_SUCCESS);
}