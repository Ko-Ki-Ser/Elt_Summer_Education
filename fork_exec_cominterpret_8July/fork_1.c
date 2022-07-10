#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main () {

	system("clear");

	pid_t ret = 0;

	ret = fork();

	if (ret != 0 && ret != -1) {
		printf ("ret is %d. It's parent! PPID = %d, PID = %d\n\n", ret,\
		 		getppid(), getpid());
		waitpid(ret, 0, 0);
		exit(EXIT_SUCCESS);
	}	
		else if (ret == 0) {
			printf ("ret is %d. It's child! PPID = %d, PID = %d\n\n", ret,\
		 			getppid(), getpid());
			exit(EXIT_SUCCESS);
		}
			else if (ret == -1) {
				perror (" Fork err ");
				exit (EXIT_FAILURE);
			}
}