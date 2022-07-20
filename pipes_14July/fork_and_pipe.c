#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


#define CHILD_STR "Hello, my parent!"
#define PARENT_STR "Hello, my child!"
#define BUFF_SIZE 128

int main (void) {

	int pipe_1, pipe_2; 
	int fd_pipe_1 [2], fd_pipe_2 [2];

	char buff_1 [BUFF_SIZE];
	char buff_2 [BUFF_SIZE];

	pid_t pid;

	// создаем два канала для обмена между родительским\
	и дочерним процессами

	pipe_1 = pipe(fd_pipe_1);

	if (pipe_1 == -1) {

		printf("Pipe_1 failed.\n");
		exit(EXIT_FAILURE);
	}

	pipe_2 = pipe(fd_pipe_2);

	if (pipe_2 == -1) {

		printf("Pipe_2 failed.\n");
		exit(EXIT_FAILURE);
	}
	
	// порождаем процесс

	pid = fork ();

	// код родителя
	if (pid != 0 && pid != -1) {
		
		strcpy(buff_1, PARENT_STR);

		// закрываем у первого канала конец для чтения, а\
		у второго конец на запись
		close(fd_pipe_1 [0]);
		close(fd_pipe_2 [1]);

		// пишем в первый канал
		write(fd_pipe_1 [1], buff_1, sizeof(buff_1));

		// читаем из второго канала
		read(fd_pipe_2 [0], buff_2, sizeof(buff_2));

		printf("I'm parent! Received the message: %s. And\
 sent the message: %s\n\n", buff_2, buff_1);

		waitpid(pid, 0, 0);
		exit(EXIT_SUCCESS);
	}
		// код потомка
		else if (pid == 0) {

			strcpy (buff_1, CHILD_STR);

			// закрываем у первого канала конец для записи, а\
			у второго конец на чтение
			close(fd_pipe_1 [1]);
			close(fd_pipe_2 [0]);

			// пишем в второй канал
			write(fd_pipe_2 [1], buff_1, sizeof(buff_1));

			// читаем из первого канала
			read(fd_pipe_1 [0], buff_2, sizeof(buff_2));

			printf("I'm child! Received the message: %s. And\
 sent the message: %s\n\n", buff_2, buff_1);
			
			exit(EXIT_SUCCESS);
		}

			else if (pid == -1) {

				printf("Fork failed.\n");
				exit(EXIT_FAILURE);
			}

	exit(EXIT_SUCCESS);
}