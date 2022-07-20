#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main (void) {

	// набор сигналов
	sigset_t set;
	int sig;

	printf("%d\n", getpid());

	// инициализация набора сигналов
	sigemptyset(&set);

	// добавляем SIGUSR1 в набор
	sigaddset(&set, SIGUSR1);

	// добавляем набор set в группу заблокированных
	sigprocmask (SIG_BLOCK, &set, NULL);

	int count = 0;

	while (count < 10) {
		sigwait(&set, &sig);
		printf("Hello! I'm proccess %d and I received SIGUSR1 %d times!\n",\
				getpid(), ++count);
	}
	exit(EXIT_SUCCESS);
}