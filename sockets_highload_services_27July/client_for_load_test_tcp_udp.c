/* программа-клиент для теста количества 
подключений к серверу */

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#define STR_CMP_TCP "tcp"
#define STR_CMP_UDP "udp"
#define SERVER_IP "127.0.0.1"
#define START_SOCKFD_ALLOC 150
#define BUFF_SIZE 128
#define CLIENT_MSG "Hello, I'am a testing client!"
#define POLL_TIMEOUT 5000

// для дескрипторов сокетов
int* socks_fd;
// для poll() в случае с udp
struct pollfd* fds;
// количество успешных поключений
int connects_count = 0;

// завершение клиента нажатием CTRL + C
void my_handler_CTRL_C (int sig);

// argv[1] - протокол (например udp), argv[2] - порт

int main(int argc, char** argv) {

	if (argc != 3) {
		printf("invalid count of arguments\n");
		exit(EXIT_FAILURE);
	}

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);	
	
	struct sockaddr_in server_base_sckadrr;
	memset(&server_base_sckadrr, 0, sizeof(struct sockaddr_in));
	socklen_t sockaddr_server_size = sizeof(struct sockaddr_in);
	// инициализируем поля нужными значениями
	server_base_sckadrr.sin_family = AF_INET;
	server_base_sckadrr.sin_port = htons(atoi(argv[2]));

	// inet_pton convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, SERVER_IP, &server_base_sckadrr.sin_addr) == -1) {
		perror(" inet_pton() error ");
		exit(EXIT_FAILURE);
	}

	// какой протокол на проверяемом сервере
	char type_transport_prot;
	if (!strcmp(argv[1], STR_CMP_TCP)) {
		type_transport_prot = 1;
	}
		else if (!strcmp(argv[1], STR_CMP_UDP)) {
			type_transport_prot = 2;
		}
			else {
				printf("incorrect argument\n");
				exit(EXIT_FAILURE);
			}	
	
	// результат подключения
	int connect_res = 0;
	// временные буферы
	char buff_input[BUFF_SIZE];
	memset(&buff_input, 0, sizeof(buff_input));
	char buff_output[BUFF_SIZE];
	strncpy(buff_output, CLIENT_MSG, sizeof(buff_output));
	// массив для дескрипторов сокетов
	int size_alloc = START_SOCKFD_ALLOC;
	socks_fd = calloc (size_alloc, sizeof(int));
	// указатель для безопасного перевыделения памяти
	int* sock_realloc_ptr = NULL;
	if (socks_fd == NULL) {
		printf("calloc failed\n");
		exit(EXIT_FAILURE);
	}

	// для poll() в случае с udp
	fds = calloc(size_alloc, sizeof(struct pollfd));	
	int poll_ret;

	// создаем новые подключения, пока не придет отказ	
	for(int i = 0;!connect_res; i++) {
		// создание сокета
		if(type_transport_prot == 1) {
			socks_fd[i] = socket(AF_INET, SOCK_STREAM, 0);
		}
			else if (type_transport_prot == 2) {
				socks_fd[i] = socket(AF_INET, SOCK_DGRAM, 0);
			}
		// проверка на ошибку при создании сокета	
		if (socks_fd[i] == -1) {
			printf("socks_fd[%d] failed, errno %d\n", i, errno);
			continue;
		}
 		
 		// подключаемся, если ошибка, выходим (для TCP)
		if (type_transport_prot == 1) {
			if (connect(socks_fd[i], (struct sockaddr*) &server_base_sckadrr,\
								sockaddr_server_size) == -1) {
				printf("connect() [%d] error, errno %d\n", i, errno);
				connect_res = 1;
				continue;		
			}
		}	

		// отправляем/принимаем данные, если успех в паре send\recv, то подключение состоялось
		if (type_transport_prot == 1) {
			if (send(socks_fd[i], buff_output, sizeof(buff_output), 0) == -1) {
				printf("send() [%d] error, errno %d\n", i, errno);								
			}			
			if (recv(socks_fd[i], &buff_input, sizeof(buff_input), 0) == -1) {
				printf("recv() [%d] error, errno %d\n", i, errno);	
				break;							
			}
				else {
					connects_count++;
					printf("recieve msg [%d]: %s\n", i, buff_input);	
					memset(&buff_input, 0, sizeof(buff_input));
				}
		}
			else if (type_transport_prot == 2) {
				if (sendto(socks_fd[i], buff_output, sizeof(buff_output), 0,\
					(struct sockaddr*) &server_base_sckadrr, sockaddr_server_size) == -1) {
					printf("sendto() [%d] error, errno %d\n", i, errno);									
				}
				
				fds[i].fd =	socks_fd[i];			
				fds[i].events = POLLIN;

				if (poll_ret = poll(&fds[i], 1, POLL_TIMEOUT) > 0) {
					if (recvfrom(socks_fd[i], &buff_input, sizeof(buff_input), 0,\
						(struct sockaddr*) &server_base_sckadrr, &sockaddr_server_size) == -1) {
						printf("recvfrom() [%d] error, errno %d\n", i, errno);																								
					}
						else {
							connects_count++;
							printf("recieve msg [%d]: %s\n", i, buff_input);	
							memset(&buff_input, 0, sizeof(buff_input));
						}
				}	
			}		

		// если заканчивается память под номера дескрипторов, перевыделяем
		if (i == size_alloc - 1) {
			break;
		}		
	}

	printf("count of connects before end = %d\n", connects_count);
	for (int i = 0; i < connects_count; i++) {
		close(socks_fd[i]);
	}

	free(fds);
	free(socks_fd);
	exit(EXIT_SUCCESS);
}

void my_handler_CTRL_C (int sig) {	

	printf("\ncount of connects before end = %d\n", connects_count);

	for (int i = 0; i < connects_count; i++) {
		close(socks_fd[i]);
	}

	free(fds);
	free(socks_fd);
	exit(EXIT_SUCCESS);
}