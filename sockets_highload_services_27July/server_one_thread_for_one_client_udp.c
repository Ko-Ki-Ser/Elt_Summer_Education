/* сервер по схеме "под каждого клиента создается свой поток" для UDP*/

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
#define SERVER_MSG "HELLO! I'AM SERVER! 'one thread for one client (udp)'"


// сущности для работы с клиентскими адресами
struct sockaddr_in* addr_clients;
socklen_t* addr_clients_size;

// переменные потоков для работы
pthread_t* threads_clients;
pthread_t thread_shutdown;

// для передачи аргументов в потоки
struct pthread_args {
	struct sockaddr_in addr_client;
	socklen_t addr_client_size;
	int struct_count;
};

struct pthread_args* args;

// переменная для принудительного завершения потоков
int stop_threads = 0;

// переменная для завершения сервера
int main_while_var = 1;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int error_flag = 0;

// количество подключенных клиентов
int clients_connected_count = 0;

// сущности для серверного сокета
int server_fd;
struct sockaddr_in addr_server;

// функция клиентского потока
void* func_thread_client (void* arg);

// функция выключения сервера
void* func_thread_shutdown_server (void* arg);

int main(void) {	

	// создаем сокет с проверкой на ошибку
	server_fd = socket (AF_INET, SOCK_DGRAM, 0);
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
	addr_clients = calloc(start_alloc_size, sizeof(struct sockaddr_in));
	addr_clients_size = calloc(start_alloc_size, sizeof(socklen_t));
	threads_clients = calloc(start_alloc_size, sizeof(pthread_t));
	args = calloc (start_alloc_size, sizeof(struct pthread_args));	

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

	// временные буферы
	char buff_input_main[BUFF_SIZE];
	memset(&buff_input_main, 0, sizeof(buff_input_main));
	char buff_output_main[BUFF_SIZE];
	memset(&buff_output_main, 0, sizeof(buff_output_main));

	// главный цикл работы сервера
	// для остановки быстро ввести 's' (работает через scanf())
	while(main_while_var) {

		// принимаем данные
		addr_clients_size[count] = sizeof(struct sockaddr_in);
		if (recvfrom(server_fd, &buff_input_main, sizeof(buff_input_main), 0,
				 (struct sockaddr*) &addr_clients[count], &addr_clients_size[count]) == -1) {
			printf(" recvfrom() main while [%d] error errno = %d\n", count, errno);
			error_flag++;			
		}
		// если успех, то создаем поток и передаем ему структуру с размером
		else {
			printf("Server: recv [%d] msg: %s\n", count, buff_input_main);
			memset(&buff_input_main, 0, sizeof(buff_input_main));

			// подготавливаем структуру к передаче
			memset(&args[count_threads], 0, sizeof(struct pthread_args));
			args[count_threads].struct_count = count;
			args[count_threads].addr_client_size = addr_clients_size[count];
			memcpy(&args[count_threads].addr_client, &addr_clients[count], sizeof(struct sockaddr_in));

			if (pthread_create(&threads_clients[count_threads], NULL, func_thread_client,\
						(void*) &args[count_threads]) != 0) {
				printf(" Create thread [%d] failed\n", count);
				error_flag++;				
			}
			else {
				count_threads++;
			}
		}
		
		count++;
		// если заканчивается память, завершаемся
		if (count == start_alloc_size - 1) {
			break;
		}		
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

	free(args);
	free(threads_clients);
	free(addr_clients_size);
	free(addr_clients);	
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
	struct pthread_args* arg_thread = (struct pthread_args*)arg;	

	// временные буферы
	char buff_input[BUFF_SIZE];
	memset(&buff_input, 0, sizeof(buff_input));
	char buff_output[BUFF_SIZE];
	strncpy(buff_output, SERVER_MSG, sizeof(buff_output));

	if (sendto(server_fd, buff_output, sizeof(buff_output), 0,\
				(struct sockaddr*) &arg_thread->addr_client,\
				 arg_thread->addr_client_size) == -1) {
		printf(" sendto() [%d] error \n", arg_thread->struct_count);
		error_flag++;
	}
	else {
		clients_connected_count++;
	}

	// цикл для работы с клиентом
	while (stop_threads != 1) {
		
		// какой нибудь обмен
		
		sleep(1);		
	}	
	
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