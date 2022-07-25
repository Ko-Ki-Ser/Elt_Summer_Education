#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MY_PORT 35555
#define LISTEN_BACKLOG 1
#define BUFF_SIZE 128
#define SERV_PART_OF_ANSWER " HELLO, I'AM SERVER!"


int main (void) {

	// дескрипторы сокетов
	int server_fd, client_fd;

	// структуры для работы с адресами
	struct sockaddr_in sockaddr_server, sockaddr_client;
	socklen_t sockaddr_client_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&sockaddr_server, 0, sizeof(struct sockaddr_in));

	// инициализируем поля нужными значениями
	sockaddr_server.sin_family = AF_INET;
	sockaddr_server.sin_port = htons(MY_PORT);
	
	// связываем адрес с сокетом
	if (bind (server_fd, (struct sockaddr*) &sockaddr_server,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind() error ");
		exit(EXIT_FAILURE);
	}

	// слушаем входящие соединения
	if (listen(server_fd, LISTEN_BACKLOG) == -1) {
		perror(" listen() error ");
		exit(EXIT_FAILURE);
	}

	// принимаем запрос на подключение
	sockaddr_client_size = sizeof(struct sockaddr_in);
	client_fd = accept (server_fd, (struct sockaddr*) &sockaddr_client,\
						&sockaddr_client_size);
	if (client_fd == -1) {
		perror(" accept() error ");
		exit(EXIT_FAILURE);
	}

	// принимаем данные 
	if (recv(client_fd, &buff_input, sizeof(buff_input), MSG_WAITALL) == -1) {
		perror(" recv() error ");
		exit(EXIT_FAILURE);
	}
	printf("Server: recv msg: %s\n", buff_input);

	// обрабатываем данные
	strncpy(buff_output, buff_input, sizeof(buff_output));
	strcat(buff_output, SERV_PART_OF_ANSWER);
	printf("Server: send msg prepared: %s\n", buff_output);

	// отправляем модифицированные данные обратно клиенту
	if (send(client_fd, buff_output, sizeof(buff_output), 0) == -1) {
		perror(" send() error ");
		exit(EXIT_FAILURE);
	}

	// засыпаем ненадолго
	sleep(2);

	// освобождение ресурсов и завершение
	close(server_fd);
	close(client_fd);	
	exit(EXIT_SUCCESS);
}