#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

#define MY_SOCK_PATH "sockspath"
#define BUFF_SIZE 128
#define CLIENT_MSG " Hello, I'am client!"

int main (void) {

	// переменные для работы
	int fd_client;
	struct sockaddr_un addr_un_server;
	socklen_t addr_un_server_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	fd_client = socket (AF_LOCAL, SOCK_DGRAM, 0);
	if (fd_client == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&addr_un_server, 0, sizeof(struct sockaddr_in));

	addr_un_server_size = sizeof(struct sockaddr_in);

	// инициализируем поля нужными значениями
	addr_un_server.sun_family = AF_LOCAL;
	strncpy(addr_un_server.sun_path, MY_SOCK_PATH,\
			sizeof(addr_un_server.sun_path) - 1);

	// отправляем данные
	strncpy(buff_output, CLIENT_MSG, sizeof(buff_output));
	printf("Client: send msg prepared: %s\n", buff_output);

	if (sendto(fd_client, buff_output, sizeof(buff_output), 0,\
		(struct sockaddr*) &addr_un_server, addr_un_server_size) == -1) {
		perror(" sendto() error ");
		exit(EXIT_FAILURE);
	}

	sleep(1);

	// принимаем ответ
	if (recvfrom(fd_client, &buff_input, sizeof(buff_input), MSG_WAITALL,\
				 NULL, NULL) == -1) {
		perror(" recvfrom() error ");
		exit(EXIT_FAILURE);
	}
	printf("Client: recv msg: %s\n", buff_input);

	// засыпаем ненадолго
	sleep(2);

	// закрываем сокет и выходим
	close(fd_client);

	exit(EXIT_SUCCESS);
}