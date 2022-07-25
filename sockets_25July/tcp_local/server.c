#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define MY_SOCK_PATH "sockspath"
#define LISTEN_BACKLOG 1
#define BUFF_SIZE 128
#define SERV_PART_OF_ANSWER " HELLO, I'AM SERVER!"


int main (void) {

	// дескрипторы сокетов
	int server_fd, client_fd;

	// структуры для работы с адресами
	struct sockaddr_un addr_un_server, addr_un_client;
	socklen_t addr_un_client_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_LOCAL, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&addr_un_server, 0, sizeof(struct sockaddr_un));

	// инициализируем поля нужными значениями
	addr_un_server.sun_family = AF_LOCAL;
	strncpy(addr_un_server.sun_path, MY_SOCK_PATH,\
			sizeof(addr_un_server.sun_path) - 1);

	// на всякий случай подстраховка для bind()
	unlink(MY_SOCK_PATH);

	// связываем адрес с сокетом
	if (bind (server_fd, (struct sockaddr*) &addr_un_server,\
		sizeof(struct sockaddr_un)) == -1 ) {
		perror(" bind() error ");
		exit(EXIT_FAILURE);
	}

	// слушаем входящие соединения
	if (listen(server_fd, LISTEN_BACKLOG) == -1) {
		perror(" listen() error ");
		exit(EXIT_FAILURE);
	}

	// принимаем запрос на подключение
	addr_un_client_size = sizeof(struct sockaddr_un);
	client_fd = accept (server_fd, (struct sockaddr*) &addr_un_client,\
						&addr_un_client_size);
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
	unlink(MY_SOCK_PATH);
	exit(EXIT_SUCCESS);
}