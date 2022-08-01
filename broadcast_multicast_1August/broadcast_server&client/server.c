// Сервер broadcast

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

	// дескрипторы сокетов
	int server_fd;
	
	struct sockaddr_in sockaddr_broadcast; 	
	socklen_t sockaddr_broadcast_size;

	sockaddr_broadcast.sin_family = AF_INET;
	sockaddr_broadcast.sin_addr.s_addr = inet_addr(argv[2]);
	sockaddr_broadcast.sin_port = htons(atoi(argv[1]));

	sockaddr_broadcast_size = sizeof(struct sockaddr_in);
	
	// буфер сообщения на отправку
	char buff_output [BUFF_SIZE];

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (server_fd == -1) {
		perror(" socket() error ");
		exit(EXIT_FAILURE);
	}

	// разрешаем broadcast
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == -1) {
		perror(" setsockopt error ");
	}

	// вводим отправляемое сообщение
	printf("Enter your broadcast message, please: ");
	fgets(buff_output, sizeof(buff_output), stdin);

	int res_send;
	if ((res_send = sendto(server_fd, buff_output,strlen(buff_output), 0,\
		(struct sockaddr*) &sockaddr_broadcast,\
		 sockaddr_broadcast_size)) == -1 ) {
		perror(" sendto error ");
	}
	else {
		printf("Sendto success! Bytes sent: %d\n", res_send);
	}
	sleep(1);

	exit(EXIT_SUCCESS);
}