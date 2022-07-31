#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MY_PORT 35556
#define BUFF_SIZE 128
#define SERV_PART_OF_ANSWER " HELLO, I'AM SERVER!"


int main (void) {

	// дескрипторы сокетов
	int server_fd;

	// структуры для работы с адресами
	struct sockaddr_in sockaddr_server; 
	struct sockaddr_in sockaddr_client;
	socklen_t sockaddr_client_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_INET, SOCK_DGRAM, 0);
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

	sockaddr_client_size = sizeof(struct sockaddr_in);

	// принимаем данные 
	if (recvfrom(server_fd, &buff_input, sizeof(buff_input), 0,
				 (struct sockaddr*) &sockaddr_client, &sockaddr_client_size) == -1) {
		perror(" recvfrom() error ");
		exit(EXIT_FAILURE);
	}
	printf("Server: recv msg: %s\n", buff_input);

	sleep(1);

	// обрабатываем данные
	strncpy(buff_output, buff_input, sizeof(buff_output));
	strcat(buff_output, SERV_PART_OF_ANSWER);
	printf("Server: send msg prepared: %s\n", buff_output);

	// отправляем модифицированные данные обратно клиенту
	if (sendto(server_fd, buff_output, sizeof(buff_output), 0,\
				(struct sockaddr*) &sockaddr_client, sockaddr_client_size) == -1) {
		perror(" sendto() error ");
		exit(EXIT_FAILURE);
	}

	// засыпаем ненадолго
	sleep(2);

	// освобождение ресурсов и завершение
	close(server_fd);
		
	exit(EXIT_SUCCESS);
}