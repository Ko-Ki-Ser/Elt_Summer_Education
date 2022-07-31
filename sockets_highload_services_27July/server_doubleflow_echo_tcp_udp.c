/* двухпоточный echo-сервер */

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>

#define SERVER_UDP_PORT 35556
#define SERVER_TCP_PORT 35555
#define BUFF_SIZE 128
#define LISTEN_BACKLOG 5
#define EXIT_BUFF_MSG "s"
#define EXIT_BUFF_SIZE 2
#define SERVER_MSG "HELLO! I'AM ECHO SERVER TCP/UDP !"
#define SEC 5
#define USEC 0

// для серверных сокетов
int fd_server_tcp, fd_server_udp;
struct sockaddr_in addr_server_tcp, addr_server_udp;

// для клиента tcp
int fd_client_tcp;

// для адресов клиентов
struct sockaddr_in addr_client_tcp, addr_client_udp;
socklen_t addr_clients_tcp_size, addr_client_udp_size;

// для потоков
pthread_t thread_client_udp;
pthread_t thread_client_tcp;
pthread_t thread_shutdown;

// для select()
fd_set fd_in;
struct timeval tv;
int largest_sock, sel_ret;

// переменная для завершения сервера
int main_while_var = 1;

// переменная для принудительного завершения потоков
int stop_threads = 0;

// буферы для сообщений
char buff_input [BUFF_SIZE];
char buff_output [BUFF_SIZE];

// функция для tcp-клиента
void* func_thread_client_tcp (void* arg);

// функция для udp-клиента
void* func_thread_client_udp (void* arg);

// функция выключения сервера
void* func_thread_shutdown_server (void* arg);

int main(void) {

	// создаем сокеты с проверкой на ошибку
	fd_server_tcp = socket (AF_INET, SOCK_STREAM, 0);
	if (fd_server_tcp == -1) {
		perror(" socket_server_tcp() error ");
		exit(EXIT_FAILURE);
	}
	fd_server_udp = socket (AF_INET, SOCK_DGRAM, 0);
	if (fd_server_udp == -1) {
		perror(" socket_server_tcp() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&addr_server_tcp, 0, sizeof(struct sockaddr_in));	
	memset(&addr_server_udp, 0, sizeof(struct sockaddr_in));

	// инициализируем поля нужными значениями
	addr_server_tcp.sin_family = AF_INET;
	addr_server_tcp.sin_port = htons(SERVER_TCP_PORT);
	addr_server_udp.sin_family = AF_INET;
	addr_server_udp.sin_port = htons(SERVER_UDP_PORT);
	
	// связываем адрес с сокетом
	if (bind (fd_server_tcp, (struct sockaddr*) &addr_server_tcp,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind_server_tcp() error ");
		exit(EXIT_FAILURE);
	}	
	if (bind (fd_server_udp, (struct sockaddr*) &addr_server_udp,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind_server_udp() error ");
		exit(EXIT_FAILURE);
	}

	// создаем поток управления выходом
	if (pthread_create(&thread_shutdown, NULL, func_thread_shutdown_server,\
						NULL) != 0) {
		printf(" Create thread_shutdown failed \n");		
	}	

	while (main_while_var) {

		// обнуляем структуру
		FD_ZERO(&fd_in);
			
		// мониторим события о входящих данных для обоих серверов
		FD_SET(fd_server_tcp, &fd_in);
		FD_SET(fd_server_udp, &fd_in);

		// таймер
		tv.tv_sec = SEC;
		tv.tv_usec = USEC;

		// определяем наибольший дескриптор
		if (fd_server_tcp > fd_server_udp) {
			largest_sock = fd_server_tcp;
		}
		else {
			largest_sock = fd_server_udp;
		}
	
		// вызываем select() с проверкой результата
		sel_ret = select(largest_sock + 1, &fd_in, NULL, NULL, &tv);

		if (sel_ret == -1) {
			printf("select failed\n");
		}
		else if (sel_ret == 0) {
			printf("timeout [%d]sec and [%d] usec is over, no events!\n", SEC, USEC);
		}
		else {

			// событие на tcp сокете
			if (FD_ISSET(fd_server_tcp, &fd_in)) {

				// создаем поток
				if (pthread_create(&thread_client_tcp, NULL, func_thread_client_tcp,\
						NULL) != 0) {
					printf(" Create thread_client_tcp failed \n");
				
				}				
				// ждем его завершения
				if (pthread_join(thread_client_tcp, NULL) != 0) {
					printf(" pthread_join thread_client_tcp failed \n");					
				}
			}
			// событие на udp сокете
			if (FD_ISSET(fd_server_udp, &fd_in)) {

				// создаем поток
				if (pthread_create(&thread_client_udp, NULL, func_thread_client_udp,\
						NULL) != 0) {
					printf(" Create thread_client_udp failed \n");
				
				}				
				// ждем его завершения
				if (pthread_join(thread_client_udp, NULL) != 0) {
					printf(" pthread_join thread_client_udp failed \n");				
				}
			}
		}
	}

	// завершение программы-сервера
	EXIT:

	stop_threads = 1;	
	
	if (pthread_join(thread_shutdown, NULL) != 0) {
		printf(" pthread_join thread_shutdown failed \n");					
	}

	close(fd_server_tcp);
	close(fd_server_udp);

	exit (EXIT_SUCCESS);
}

void* func_thread_client_tcp (void* arg) {

	// слушаем входящие соединения
	if (listen(fd_server_tcp, LISTEN_BACKLOG) == -1) {
		printf(" listen_server_tcp() error\n");						
	}
	// принимаем запрос на подключение
	addr_clients_tcp_size = sizeof(struct sockaddr_in);
	fd_client_tcp = accept (fd_server_tcp, (struct sockaddr*) &addr_client_tcp,\
						&addr_clients_tcp_size);
	if (fd_client_tcp == -1) {
		perror(" accept tcp client() error ");
		printf("\n");		
	}

	// принимаем данные 
	if (recv(fd_client_tcp, &buff_input, sizeof(buff_input), MSG_WAITALL) == -1) {
		perror(" recv client tcp() error ");
		printf("\n");		
	}
	printf("Server TCP: recv msg: %s\n", buff_input);

	// отвечаем
	strncpy(buff_output, SERVER_MSG, sizeof(buff_output));	
	printf("Server TCP: send msg prepared: %s\n", buff_output);

	if (send(fd_client_tcp, buff_output, sizeof(buff_output), 0) == -1) {
		perror(" send() error ");
		printf("\n");		
	}
	
	memset(buff_input, 0, sizeof(buff_input));
	memset(buff_output, 0, sizeof(buff_output));
	sleep(1);
	close(fd_client_tcp);
	memset(&addr_client_tcp, 0, sizeof(struct sockaddr_in));
	pthread_exit(NULL);
}

void* func_thread_client_udp (void* arg) {

	addr_client_udp_size = sizeof(struct sockaddr_in);

	// принимаем данные 
	if (recvfrom(fd_server_udp, &buff_input, sizeof(buff_input), 0,
				 (struct sockaddr*) &addr_client_udp, &addr_client_udp_size) == -1) {
		perror(" recvfrom udp client() error ");
		printf("\n");
	}
	printf("Server UDP: recvfrom msg: %s\n", buff_input);

	// отвечаем
	strncpy(buff_output, SERVER_MSG, sizeof(buff_output));	
	printf("Server UDP: sendto msg prepared: %s\n", buff_output);

	if (sendto(fd_server_udp, buff_output, sizeof(buff_output), 0,\
				(struct sockaddr*) &addr_client_udp, addr_client_udp_size) == -1) {
		perror(" sendto() error ");
		printf("\n");		
	}
	
	memset(buff_input, 0, sizeof(buff_input));
	memset(buff_output, 0, sizeof(buff_output));
	memset(&addr_client_udp, 0, sizeof(struct sockaddr_in));
	sleep(1);
	pthread_exit(NULL);
}

void* func_thread_shutdown_server (void* arg) {

	char buff_exit[EXIT_BUFF_SIZE];
	memset(buff_exit, 0, sizeof(buff_exit));

	while(stop_threads != 1) {

		scanf("%s", buff_exit);
		printf("%s\n", buff_exit);
		if (!strcmp(buff_exit, EXIT_BUFF_MSG)) {
			main_while_var = 0;
			break;
		}
		else {
			memset(buff_exit, 0, sizeof(buff_exit));
		}
	}
	pthread_exit(NULL);
}