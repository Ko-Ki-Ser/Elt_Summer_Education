#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#define MY_SOCK_PATH "sockspath"
#define BUFF_SIZE 128
#define SERV_PART_OF_ANSWER " HELLO, I'AM SERVER!"

int main (void) {

	// дескриптор сокета
	int server_fd;

	// структуры для работы с адресами
	struct sockaddr_un sockaddr_server;
	struct sockaddr_un sockaddr_client;
	socklen_t sockaddr_client_size;

	// буферы для сообщений
	char buff_input [BUFF_SIZE];
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_LOCAL, SOCK_DGRAM, 0);
	if (server_fd == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&sockaddr_server, 0, sizeof(struct sockaddr_un));	

	// инициализируем поля нужными значениями
	sockaddr_server.sun_family = AF_LOCAL;
	strncpy(sockaddr_server.sun_path, MY_SOCK_PATH,\
			sizeof(sockaddr_server.sun_path) - 1);

	// на всякий случай подстраховка для bind()
	unlink(MY_SOCK_PATH);

	// связываем адрес с сокетом
	if (bind (server_fd, (struct sockaddr*) &sockaddr_server,\
		sizeof(struct sockaddr)) == -1 ) {
		perror(" bind() error ");
		exit(EXIT_FAILURE);
	}	

	sockaddr_client_size = sizeof(struct sockaddr_un);
	
	// принимаем данные 
	if (recvfrom(server_fd, &buff_input, sizeof(buff_input), MSG_WAITALL,\
				(struct sockaddr*) &sockaddr_client,\
				 &sockaddr_client_size) == -1) {
		perror(" recvfrom() error ");
		exit(EXIT_FAILURE);
	}
	printf("Server: recvfrom msg: %s\n", buff_input);	

	sleep(1);

	// обрабатываем данные
	strncpy(buff_output, buff_input, sizeof(buff_output));
	strcat(buff_output, SERV_PART_OF_ANSWER);
	printf("Server: send msg prepared: %s\n", buff_output);

	// sendto Transport endpoint is not connected
	// errno 107

	// отправляем модифицированные данные обратно клиенту
	if (sendto(server_fd, buff_output, sizeof(buff_output), 0,\
		(struct sockaddr*) &sockaddr_client, sockaddr_client_size) == -1) {
		perror(" sendto() error ");
		printf("\nERRNO:%d\n", errno);
		exit(EXIT_FAILURE);
	}

	// засыпаем ненадолго
	sleep(2);

	// освобождение ресурсов и завершение
	close(server_fd);	
	unlink(MY_SOCK_PATH);

	exit(EXIT_SUCCESS);
}