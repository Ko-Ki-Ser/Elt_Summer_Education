// Клиент broadcast

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFF_SIZE 128

// через argv[1] получаем порт, через argv[2] - IPv4
int main (int argc, char** argv) {

	if (argc != 3) {
		printf("Invalid argc\n");
		exit(EXIT_FAILURE);
	}

	// переменные для работы
	int fd_client;
	struct sockaddr_in sockaddr_server;
	socklen_t sockaddr_server_size;

	// инициализируем поля нужными значениями
	sockaddr_server.sin_family = AF_INET;
	sockaddr_server.sin_addr.s_addr = inet_addr(argv[2]);
	sockaddr_server.sin_port = htons(atoi(argv[1]));

	sockaddr_server_size = sizeof(struct sockaddr_in);

	// буфер для сообщения
	char buff_input [BUFF_SIZE];	

	// создаем сокет с проверкой на ошибку
	fd_client = socket (AF_INET, SOCK_DGRAM, 0);
	if (fd_client == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// связываем адрес с сокетом
	if (bind (fd_client, (struct sockaddr*) &sockaddr_server,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind() error ");
		close(fd_client);
		exit(EXIT_FAILURE);
	}

	// принимаем данные 
	if (recvfrom(fd_client, &buff_input, sizeof(buff_input), 0,
				 (struct sockaddr*) &sockaddr_server, &sockaddr_server_size) == -1) {
		perror(" recvfrom() error ");
		close(fd_client);
		exit(EXIT_FAILURE);
	}
	printf("Recv msg: %s", buff_input);

	sleep(1);

	exit(EXIT_SUCCESS);
}