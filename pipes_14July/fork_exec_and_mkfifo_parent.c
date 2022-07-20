#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PATH_AND_NAME_1 "/home/kirill/github repositories/Eltex_Education/\
pipes_14July/fifo_1.1"

#define PATH_AND_NAME_2 "/home/kirill/github repositories/Eltex_Education/\
pipes_14July/fifo_2.2"

#define CHILD_EXEC_NAME "/home/kirill/github repositories/Eltex_Education/\
pipes_14July/fork_exec_and_mkfifo_child.out"

#define MESSAGE_OUT "Hello! I am parent! Using mkfifo_2 to write, mkfifo_1 to read"

#define BUFF_SIZE 128

int main (void) {

	int mkfifo_1_res, mkfifo_2_res;
	int fd_fifo_1, fd_fifo_2;
	pid_t pid;
	char buff_in [BUFF_SIZE];
	char buff_out [BUFF_SIZE] = {MESSAGE_OUT};
	
	// для вызова execv
	char** args = malloc(sizeof(void*) * 4);
	for (int i = 0; i < 3; i++) {
		args[i] = malloc(BUFF_SIZE);
	}
	
	memcpy(args[0], CHILD_EXEC_NAME, strlen(CHILD_EXEC_NAME));
	memcpy(args[1], PATH_AND_NAME_1, strlen(PATH_AND_NAME_1));
	memcpy(args[2], PATH_AND_NAME_2, strlen(PATH_AND_NAME_2));
	args[3] = NULL;

	// если файлы существуют, удалим их перед созданием новых
	unlink(PATH_AND_NAME_1);
	unlink(PATH_AND_NAME_2);

	
	// создание двух именованных каналов с проверкой возможной ошибки
	// 0777 (rwxrwxrwx)
	mkfifo_1_res = mkfifo (PATH_AND_NAME_1, 0777);

	if (mkfifo_1_res == -1) {
		perror(" mkfifo_1 ");
		exit(EXIT_FAILURE);
	}

	mkfifo_2_res = mkfifo (PATH_AND_NAME_2, 0777);

	if (mkfifo_2_res == -1) {
		perror(" mkfifo_2 ");
		exit(EXIT_FAILURE);
	}
	

	// создание дочернего процесса
	pid = fork ();

	// родительская ветка
	if (pid != -1 && pid != 0) {

		// открываем специализированные файлы каналов 
		fd_fifo_1 = open (PATH_AND_NAME_1, O_RDONLY);

		if (fd_fifo_1 == -1) {
			perror(" open mkfifo_1 parent ");
			exit(EXIT_FAILURE);
		}

		fd_fifo_2 = open (PATH_AND_NAME_2, O_WRONLY);

		if (fd_fifo_2 == -1) {
			perror(" open mkfifo_2 parent ");
			exit(EXIT_FAILURE);
		}

		// пишем и читаем с проверкой возможных ошибок
		if (write(fd_fifo_2, buff_out, sizeof(buff_out)) == -1) {
			perror(" write parent failed ");
			exit(EXIT_FAILURE);
		}

		sleep(1);

		if (read(fd_fifo_1, buff_in, sizeof(buff_in)) == -1) {
			perror(" read parent failed ");
			exit(EXIT_FAILURE);
		}

		// выводим отправленное и принятое сообщения
		printf("\nPARENT IN: %s\nPARENT OUT: %s\n", buff_in, buff_out);

		// ожидаем завершения потомка
		waitpid(pid, 0, 0);

		for (int i = 0; i < 3; i++) {
			free(args[i]);
		}
		free (args);
	}
		// дочернего процесса ветка
		else if (pid == 0) {

			if (execv(args[0], args) == -1) {
      			
      			perror(" execv ");
      			for (int i = 0; i < 3; i++) {
					free(args[i]);
				}
				free (args);
      			exit(EXIT_FAILURE);
    		}

		}
			// обработка ошибки
			else if (pid == -1) {
				perror (" fork err ");
				for (int i = 0; i < 3; i++) {
					free(args[i]);
				}
				free (args);
				exit (EXIT_FAILURE);
			}
	
	exit(EXIT_SUCCESS);
}