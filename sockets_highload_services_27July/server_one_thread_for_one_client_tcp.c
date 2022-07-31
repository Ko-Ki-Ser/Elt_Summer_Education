/* сервер по схеме "под каждого клиента создается свой поток" для TCP*/

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

#define SERVER_PORT 35555
#define START_ALLOC 100
#define BUFF_SIZE 128
#define LISTEN_BACKLOG 5
#define EXIT_BUFF_MSG "s"
#define EXIT_BUFF_SIZE 2
#define SERVER_MSG "HELLO! I'AM SERVER! 'one thread for one client (tcp)'"


// дескрипторы клиентских сокетов
int* clients_fd;

// сущности для работы с клиентскими адресами
struct sockaddr_in* addr_clients;
socklen_t* addr_clients_size;

// переменные потоков для работы
pthread_t* threads_clients;
pthread_t thread_shutdown;

// для передачи аргументов в потоки
int* pthread_args;

// переменная для принудительного завершения потоков
int stop_threads = 0;

// переменная для завершения сервера
int main_while_var = 1;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int error_flag = 0;

// количество подключенных клиентов
int clients_connected_count = 0;

// функция клиентского потока
void* func_thread_client (void* arg);

// функция выключения сервера
void* func_thread_shutdown_server (void* arg);

int main(void) {

	// сущности для серверного сокета
	int server_fd;
	struct sockaddr_in addr_server;

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror(" socket_server() error ");
		exit(EXIT_FAILURE);
	}

	// зануляем структуру
	memset(&addr_server, 0, sizeof(struct sockaddr_in));

	// инициализируем поля нужными значениями
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(SERVER_PORT);

	// связываем адрес с сокетом
	if (bind (server_fd, (struct sockaddr*) &addr_server,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind_server() error ");
		exit(EXIT_FAILURE);
	}

	// выделение памяти
	int start_alloc_size = START_ALLOC;
	clients_fd = calloc(start_alloc_size, sizeof(int));
	addr_clients = calloc(start_alloc_size, sizeof(struct sockaddr_in));
	addr_clients_size = calloc(start_alloc_size, sizeof(socklen_t));
	threads_clients = calloc(start_alloc_size, sizeof(pthread_t));
	pthread_args = calloc (start_alloc_size, sizeof(int));	

	// счетчики для цикла
	// основной
	int count = 0;
	// для потоков
	int count_threads = 0;
	// для клиентских дескрипторов
	int count_clients_fd = 0;

	// создаем поток управления выходом
	if (pthread_create(&thread_shutdown, NULL, func_thread_shutdown_server,\
						NULL) != 0) {
		printf(" Create thread_shutdown failed");
		error_flag++;
		goto EXIT;
	}	

	// главный цикл работы сервера
	// для остановки быстро ввести 's' (работает через scanf())
	while(main_while_var) {

		// слушаем входящие соединения
		if (listen(server_fd, LISTEN_BACKLOG) == -1) {
			printf(" listen_server() error (count = %d) \n", count);
			error_flag++;			
		}
		else {
			// принимаем запрос на подключение
			addr_clients_size[count] = sizeof(struct sockaddr_in);
			clients_fd[count] = accept (server_fd,\
								(struct sockaddr*) &addr_clients[count],\
								 &addr_clients_size[count]);
			if (clients_fd[count] == -1) {
				printf(" accept() error (count = %d) \n", count);
				error_flag++;
			}
			else {
				count_clients_fd++;
				pthread_args[count_threads] = count;
				if (pthread_create(&threads_clients[count_threads], NULL, func_thread_client,\
								  (void*) &pthread_args[count_threads]) != 0) {
					printf(" Create thread failed (count = %d) \n", count);
					error_flag++;							
				}
				else {
					count_threads++;
				}							
			}				
		}
		count++;

		// если заканчивается память, завершаемся
		if (count == start_alloc_size - 1) {
			break;
		}
		sleep(1);
	}	

	// завершение программы-сервера
	EXIT:

	stop_threads = 1;
	// ожидание завершения потоков
	for (int i = 0; i < count_threads; i++) {
		if (pthread_join(threads_clients[i], NULL) != 0) {
			printf(" pthread_join threads_clients[%d] failed \n", i);
			error_flag++;			
		}
	}

	if (pthread_join(thread_shutdown, NULL) != 0) {
		printf(" pthread_join thread_shutdown failed \n");
		error_flag++;			
	}

	free(pthread_args);
	free(threads_clients);
	free(addr_clients_size);
	free(addr_clients);
	free(clients_fd);
	close(server_fd);

	printf("clients_connected_count = %d\n", clients_connected_count);

	if (error_flag == 0) {
		exit (EXIT_SUCCESS);
	}
	else {
		exit(EXIT_FAILURE);
	}
}

void* func_thread_client (void* arg) {

	// принятый аргумент
	int* arg_count = (int*) arg;

	// временные буферы
	char buff_input[BUFF_SIZE];
	memset(&buff_input, 0, sizeof(buff_input));
	char buff_output[BUFF_SIZE];
	strncpy(buff_output, SERVER_MSG, sizeof(buff_output));

	// отправляем/принимаем данные, если успех в паре recv/send,
	// то подключение состоялось				
	if (recv(clients_fd[*arg_count], &buff_input, sizeof(buff_input), MSG_WAITALL) == -1) {
		printf("recv() [%d] error, errno %d\n", *arg_count, errno);						
	}
	if (send(clients_fd[*arg_count], buff_output, sizeof(buff_output), 0) == -1) {
		printf("send() [%d] error, errno %d\n", *arg_count, errno);						
	}
	else {
		clients_connected_count++;
		printf("recieve msg [%d]: %s\n", *arg_count, buff_input);
		printf("send msg [%d]: %s\n", *arg_count, buff_output);	
		memset(&buff_input, 0, sizeof(buff_input));
		memset(&buff_output, 0, sizeof(buff_output));
	}

	
	ssize_t res_recv = 0;
	// цикл для работы с клиентом
	while (stop_threads != 1) {

		// тут какой-нибудь обмен

		if (recv(clients_fd[*arg_count], &buff_input, sizeof(buff_input), MSG_DONTWAIT) == -1) {
			printf("recv() [%d] error, errno %d\n", *arg_count, errno);						
		}
		// условие завершения работы: если клиент отключился, то завершаемся
		if (res_recv == 0) {
			printf("client [%d] disconnected\n", *arg_count);
			break;					
		}
		printf("recieve msg [%d]: %s\n", *arg_count, buff_input);
		memset(&buff_input, 0, sizeof(buff_input));		
	}

	close(clients_fd[*arg_count]);
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