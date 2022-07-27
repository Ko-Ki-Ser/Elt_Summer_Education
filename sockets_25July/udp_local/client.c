#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

#define MY_SOCK_PATH_SERV "sockspathserv"
#define MY_SOCK_PATH_CLI "sockspathcli"
#define BUFF_SIZE 128
#define CLIENT_MSG " Hello, I'am client!"

int main (void) {

	// переменные для работы
	int fd_client;
	struct sockaddr_un addr_un_server;
	struct sockaddr_un addr_un_client;
	socklen_t addr_un_server_size;
	socklen_t addr_un_client_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	fd_client = socket (AF_LOCAL, SOCK_DGRAM, 0);
	if (fd_client == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуры
	memset(&addr_un_server, 0, sizeof(struct sockaddr_un));	
	memset(&addr_un_client, 0, sizeof(struct sockaddr_un));

	addr_un_server_size = sizeof(struct sockaddr_un);
	addr_un_client_size = sizeof(struct sockaddr_un);

	// инициализируем поля нужными значениями
	addr_un_server.sun_family = AF_LOCAL;
	strncpy(addr_un_server.sun_path, MY_SOCK_PATH_SERV,\
			sizeof(addr_un_server.sun_path) - 1);
	addr_un_client.sun_family = AF_LOCAL;
	strncpy(addr_un_client.sun_path, MY_SOCK_PATH_CLI,\
			sizeof(addr_un_client.sun_path) - 1);

	// на всякий случай подстраховка для bind()
	unlink(MY_SOCK_PATH_CLI);

	// связываем адрес с сокетом
	if (bind (fd_client, (struct sockaddr*) &addr_un_client,\
		sizeof(struct sockaddr)) == -1 ) {
		perror(" bind() error ");
		exit(EXIT_FAILURE);
	}	

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
	unlink(MY_SOCK_PATH_CLI);

	exit(EXIT_SUCCESS);
}