/* сервер по схеме "сервер при запуске 
создает пулл потоков для работы с клиентами" */

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
#include <signal.h>

#define SERVER_PORT 35555
#define START_ALLOC 100
#define BUFF_SIZE 128
#define LISTEN_BACKLOG 5
#define EXIT_BUFF_MSG "s"
#define EXIT_BUFF_SIZE 2
#define SERVER_MSG "HELLO! I'AM SERVER! 'pull of threads'"

// дескрипторы клиентских сокетов
int* clients_fd;

// сущности для работы с клиентскими адресами
struct sockaddr_in* addr_clients;
socklen_t* addr_clients_size;

// переменные потоков для работы
pthread_t* threads_clients;
pthread_t thread_shutdown;

// для передачи аргументов в потоки
struct pthread_args {	
	int struct_count;
	int client_fd;
	int client_fd_no;
	int flag;
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

// массив мьютексов для блокировок
pthread_mutex_t* ml;

// сущности для серверного сокета
int server_fd;
struct sockaddr_in addr_server;

int start_alloc_size;

// функция клиентского потока
void* func_thread_client (void* arg);

// функция выключения сервера
void* func_thread_shutdown_server (void* arg);

// завершение сервера нажатием CTRL + C (нестабильное)
void my_handler_CTRL_C (int sig);

int main(void) {

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);	

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

	// выделяем необходимое количество памяти
	start_alloc_size = START_ALLOC;
	ml = calloc (start_alloc_size, sizeof(pthread_mutex_t));
	clients_fd = calloc(start_alloc_size, sizeof(int));
	addr_clients = calloc(start_alloc_size, sizeof(struct sockaddr_in));
	addr_clients_size = calloc(start_alloc_size, sizeof(socklen_t));
	threads_clients = calloc(start_alloc_size, sizeof(pthread_t));
	args = calloc (start_alloc_size, sizeof(struct pthread_args));

	// создаем поток управления выходом
	if (pthread_create(&thread_shutdown, NULL, func_thread_shutdown_server,\
						NULL) != 0) {
		printf(" Create thread_shutdown failed");
		error_flag++;
		goto EXIT;
	}	

	// инициализация мьютексов
	for (int i = 0; i < start_alloc_size; i++) {
		pthread_mutex_init(&ml[i], NULL);
	}

	// создаем пулл потоков для работы с клиентами
	for (int i = 0; i < start_alloc_size; i++) {

		args[i].flag = 0;
		args[i].struct_count = i;
		if (pthread_create(&threads_clients[i], NULL, func_thread_client,\
						&args[i]) != 0) {
			printf(" Create thread_[%d] failed \n", i);
			error_flag++;			
		}
	}
	
	// для выбора сокета
	int clients_fd_no = 0;

	int count = 0;	

	// главный цикл работы сервера
	// для остановки быстро ввести 's' (работает через fgets())
	while(main_while_var) {

		// проверяем освобожденные переменные сокетов и зануляем их
		for (int i = 0; i < start_alloc_size; i++) {
			pthread_mutex_lock(&ml[i]);
			if ((args[i].flag == 0) && (args[i].client_fd != 0)) {
				clients_fd[args[i].client_fd_no] = 0;
			}
			pthread_mutex_unlock(&ml[i]);
		}

		// слушаем входящие соединения на теперь уже пассивном сокете сервера
		if (listen(server_fd, LISTEN_BACKLOG) == -1) {
			printf(" listen_server() error (count = %d) \n", count);
			error_flag++;			
		}
		else {
			
			// ищем свободную переменную для сокета
			for (int i = 0; i < start_alloc_size; i++) {
				if (clients_fd[i] == 0) {
					clients_fd_no = i;
					break;
				}
			}
			// принимаем запрос на подключение			
			addr_clients_size[clients_fd_no] = sizeof(struct sockaddr_in);
			clients_fd[clients_fd_no] = accept (server_fd,\
								(struct sockaddr*) &addr_clients[clients_fd_no],\
								 &addr_clients_size[clients_fd_no]);
			
			if (clients_fd[clients_fd_no] == -1) {
				printf(" accept() error (clients_fd_no = %d) \n", clients_fd_no);
				error_flag++;
			}
			else {				
				// ищем свободный поток
				for (int i = 0; i < start_alloc_size; i++) {					
					// критическая секция
					pthread_mutex_lock(&ml[i]);
					// когда находим передаем ему клиентский сокет
					if (args[i].flag == 0) {
						args[i].client_fd = clients_fd[clients_fd_no];
						args[i].client_fd_no = clients_fd_no;
						args[i].flag = 1;
						pthread_mutex_unlock(&ml[i]);
						break;
					}
					pthread_mutex_unlock(&ml[i]);
				}
			}
		}
		printf("clients_connected_count = %d\n", clients_connected_count);
		count++;
		clients_fd_no = 0;		

		if (count == start_alloc_size) {
			break;
		}

	}

	// завершение работы
	EXIT:

	stop_threads = 1;

	// ожидание завершения потоков
	for (int i = 0; i < start_alloc_size; i++){
		if (pthread_join(threads_clients[i], NULL) != 0) {
			printf(" pthread_join threads_clients[%d] failed \n", i);
			error_flag++;			
		}
	}

	if (pthread_join(thread_shutdown, NULL) != 0) {
		printf(" pthread_join thread_shutdown failed \n");
		error_flag++;			
	}	

	// проверяем, все ли клиентские сокеты закрыты
	for (int i = 0; i < start_alloc_size; i++) {
		if (clients_fd[i] != 0) {
			close(clients_fd[i]);
		}
	}

	free(args);
	free(threads_clients);
	free(addr_clients_size);
	free(addr_clients);
	free(clients_fd);
	// уничтожение мьютексов при завершении
	for (int i = 0; i < start_alloc_size; i++) {
		pthread_mutex_destroy(&ml[i]);		
	}
	free(ml);
	close(server_fd);

	printf("clients_connected_count in the end = %d\n", clients_connected_count);

	if (error_flag == 0) {
		exit (EXIT_SUCCESS);
	}
	else {
		exit (EXIT_FAILURE);
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

	while (stop_threads != 1) {

		// если флажок = 1, то нам дали клиента и его дескриптор
		if (arg_thread->flag == 1) {

			// отправляем/принимаем данные, если успех в паре recv/send,
			// то подключение состоялось				
			if (recv(arg_thread->client_fd, &buff_input, sizeof(buff_input), MSG_WAITALL) == -1) {
				printf("recv() [%d] error, errno %d\n", arg_thread->struct_count, errno);						
			}
			if (send(arg_thread->client_fd, buff_output, sizeof(buff_output), 0) == -1) {
				printf("send() [%d] error, errno %d\n", arg_thread->struct_count, errno);						
			}
			else {
				clients_connected_count++;
				printf("recieve msg [%d]: %s\n", arg_thread->struct_count, buff_input);
				printf("send msg [%d]: %s\n", arg_thread->struct_count, buff_output);	
				memset(&buff_input, 0, sizeof(buff_input));
				memset(&buff_output, 0, sizeof(buff_output));

				// цикл обмена с клиентом
				while (stop_threads != 1) {

					// тут какой-нибудь обмен

					sleep(1);					

					// завершение работы с клиентом
					pthread_mutex_lock(&ml[arg_thread->struct_count]);
					arg_thread->flag = 0;
					printf("client [%d] disconnected\n", arg_thread->struct_count);
					close(arg_thread->client_fd);																
					clients_connected_count--;
					pthread_mutex_unlock(&ml[arg_thread->struct_count]);
					break;					
				}
			}
		}
		sleep(1);		
	}	
	pthread_exit(NULL);
}

void my_handler_CTRL_C (int sig) {
	main_while_var = 0;
	stop_threads = 1;
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