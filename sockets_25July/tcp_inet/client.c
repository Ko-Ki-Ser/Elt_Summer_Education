#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MY_SERVER_PORT 35555
#define MY_SERVER_IPV4_ADDR "127.0.0.1"
#define BUFF_SIZE 128
#define CLIENT_MSG " Hello, I'am client!"

int main (void) {

	// переменные для работы
	int fd_client;
	struct sockaddr_in sockaddr_server;
	socklen_t sockaddr_server_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	fd_client = socket (AF_INET, SOCK_STREAM, 0);
	if (fd_client == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&sockaddr_server, 0, sizeof(struct sockaddr_in));

	// инициализируем поля нужными значениями
	sockaddr_server.sin_family = AF_INET;
	sockaddr_server.sin_port = htons(MY_SERVER_PORT);

	// inet_pton convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, MY_SERVER_IPV4_ADDR, &sockaddr_server.sin_addr) == -1) {
		perror(" inet_pton() error ");
		exit(EXIT_FAILURE);
	}	
	
	// подключаемся к серверу
	sockaddr_server_size = sizeof(struct sockaddr_in);
	if (connect(fd_client, (struct sockaddr*) &sockaddr_server,\
							sockaddr_server_size) == -1) {
		perror(" connect() error ");
		exit(EXIT_FAILURE);
	}

	// отправляем данные
	strncpy(buff_output, CLIENT_MSG, sizeof(buff_output));
	printf("Client: send msg prepared: %s\n", buff_output);
	if (send(fd_client, buff_output, sizeof(buff_output), 0) == -1) {
		perror(" send() error ");
		exit(EXIT_FAILURE);
	}

	// принимаем ответ
	if (recv(fd_client, &buff_input, sizeof(buff_input), MSG_WAITALL) == -1) {
		perror(" recv() error ");
		exit(EXIT_FAILURE);
	}
	printf("Client: recv msg: %s\n", buff_input);

	// засыпаем ненадолго
	sleep(2);

	// закрываем сокет и выходим
	close(fd_client);
	exit(EXIT_SUCCESS);
}