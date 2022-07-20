#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define MESSAGE_OUT "Hello! I am child! Using mkfifo_1 to write, mkfifo_2 to read"

int main (int argc, char** argv) {

	int fd_fifo_1, fd_fifo_2;
	char buff_in [128];
	char buff_out [128] = {MESSAGE_OUT};

	// открываем специализированные файлы каналов 
	fd_fifo_1 = open (argv[1], O_WRONLY);

	if (fd_fifo_1 == -1) {
		perror(" open mkfifo_1 child ");
		exit(EXIT_FAILURE);
	}

	fd_fifo_2 = open (argv[2], O_RDONLY);

	if (fd_fifo_2 == -1) {
		perror(" open mkfifo_2 child ");
		exit(EXIT_FAILURE);
	}
	
	// пишем и читаем с проверкой возможных ошибок
	if (write(fd_fifo_1, buff_out, sizeof(buff_out)) == -1) {
		perror(" write child failed ");
		exit(EXIT_FAILURE);
	}

	sleep(1);

	if (read(fd_fifo_2, buff_in, sizeof(buff_in)) == -1) {
		perror(" read child failed ");
		exit(EXIT_FAILURE);
	}

	// выводим отправленное и принятое сообщения
	printf("\nCHILD IN: %s\nCHILD OUT: %s\n", buff_in, buff_out);

	exit(EXIT_SUCCESS);
}