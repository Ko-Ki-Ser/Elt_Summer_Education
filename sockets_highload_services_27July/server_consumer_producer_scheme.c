/* сервер по схеме 'производитель - потребитель' для TCP */

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
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>


#define SERVER_TCP_PORT 35555
#define BUFF_SIZE 128
#define LISTEN_BACKLOG 5
// количество обслуживаемых клиентов
#define START_ALLOC 50

#define EXIT_BUFF_MSG "s"
#define EXIT_BUFF_SIZE 1
#define SERVER_MSG "Hello! I'am thread no:"

#define QUE_NAME "/myqueue"
#define PRIORITY 1
#define MAX_MSG 10

// для select()
#define SEC 5
#define USEC 0

// дескрипторы клиентских сокетов
int* clients_fd;

// сущности для работы с клиентскими адресами
struct sockaddr_in* addr_clients;
socklen_t* addr_clients_size;

// сущности для серверного сокета
int server_fd;
struct sockaddr_in addr_server;

int start_alloc_size = START_ALLOC;

// переменные потоков для работы
pthread_t* threads_clients;
pthread_t thread_input;
pthread_t thread_count_connected_clients;

// для передачи аргументов в потоки
struct pthread_args {	
	int thread_no;
	int client_fd;	
	int flag_connect;
};

struct pthread_args* args;

// массив мьютексов для блокировок структур аргументов потоков
pthread_mutex_t* ml_thread;

// переменная для очереди
mqd_t que;

// для параметров очереди
struct mq_attr que_attr;

// переменная для принудительного завершения потоков
int stop_threads = 0;

// переменная для завершения сервера
int main_while_var = 1;

// количество подключенных клиентов
int clients_connected_count = 0;

// временные буферы
char buff_input_keyboard [BUFF_SIZE];
char buff_input_for_work [BUFF_SIZE];
char msg_to_que_buff [BUFF_SIZE];

// для select()
fd_set fd_in;
struct timeval tv;
int sel_ret;

// функция клиентского потока
void* func_thread_client (void* arg);

// функция потока ввода данных с условием завершения 
// работы сервера
void* func_thread_input_server (void* arg);

// функция для потока подсчета подключенных клиентов
void* func_thread_count_connected_clients (void* arg);

int main(void) {

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
	addr_server.sin_port = htons(SERVER_TCP_PORT);

	// связываем адрес с сокетом
	if (bind (server_fd, (struct sockaddr*) &addr_server,\
		sizeof(struct sockaddr_in)) == -1 ) {
		perror(" bind_server() error ");
		close (server_fd);
		exit(EXIT_FAILURE);
	}

	// выделяем память
	clients_fd = calloc(start_alloc_size, sizeof(int));
	addr_clients = calloc(start_alloc_size, sizeof(struct sockaddr_in));
	addr_clients_size = calloc(start_alloc_size, sizeof(socklen_t));
	threads_clients = calloc(start_alloc_size, sizeof(pthread_t));
	args = calloc(start_alloc_size, sizeof(struct pthread_args));
	ml_thread = calloc(start_alloc_size, sizeof(pthread_mutex_t));

	// инициализация мьютексов	
	for (int i = 0; i < start_alloc_size; i++) {
		if (pthread_mutex_init(&ml_thread[i], NULL) != 0) {
			printf(" pthread_mutex_init failed \n");				
		}
	}

	// создание очереди сообщений
	que_attr.mq_maxmsg = MAX_MSG;
	que_attr.mq_msgsize = BUFF_SIZE;
	if ((que = mq_open(QUE_NAME, O_CREAT|O_RDWR|O_NONBLOCK , 0777,\
					   &que_attr)) == (mqd_t)-1) {
    	perror(" creating queue failed ");		
	}

	// создаем пулл клиентских потоков
	for (int i = 0; i < start_alloc_size; i++) {

		args[i].flag_connect = 0;
		args[i].thread_no = i;
		args[i].client_fd = 0;
		if (pthread_create(&threads_clients[i], NULL, func_thread_client,\
						&args[i]) != 0) {
			printf(" Create thread_[%d] failed \n", i);						
		}
	}

	// создаем поток управления вводом
	if (pthread_create(&thread_input, NULL, func_thread_input_server,\
						NULL) != 0) {
		printf(" Create thread_input failed\n");		
	}

	// создаем поток подсчета подключенных клиентов
	if (pthread_create(&thread_count_connected_clients, NULL, func_thread_count_connected_clients,\
						NULL) != 0) {
		printf(" Create thread_count_connected_clients failed\n");		
	}

	if (listen(server_fd, LISTEN_BACKLOG) == -1) {
		printf(" listen_server() error \n");				
	}

	int count = 0;
	// главный цикл работы сервера
	while (main_while_var) {
		
		// обнуляем структуру
		FD_ZERO(&fd_in);			
		
		FD_SET(server_fd, &fd_in);		

		// таймер
		tv.tv_sec = SEC;
		tv.tv_usec = USEC;

		// вызываем select() с проверкой результата
		sel_ret = select(server_fd + 1, &fd_in, NULL, NULL, &tv);

		if (sel_ret == -1) {
			printf("select failed\n");
		}
		else if (sel_ret == 0) {
			printf("timeout [%d]sec and [%d] usec is over, no events!\n", SEC, USEC);
		}
		else if (FD_ISSET(server_fd, &fd_in)) {				
			
			addr_clients_size[count] = sizeof(struct sockaddr_in);
			clients_fd[count] = accept (server_fd, (struct sockaddr*) &addr_clients[count],\
						 &addr_clients_size[count]);
			if (clients_fd[count] == -1) {
				printf(" accept() error (count = %d) \n", count);
			}
			else {
				// и кладем count в очередь
				memset(msg_to_que_buff, 0, sizeof(msg_to_que_buff));
				sprintf(msg_to_que_buff, "%d", count);
printf("msg_to_que_buff %s\n", msg_to_que_buff);				
				mq_send(que, msg_to_que_buff, strlen(msg_to_que_buff), PRIORITY);
				
				count++;

				// если достигли предела
				if (count == start_alloc_size - 1) {
					printf("count == start_alloc_size\n");
					// повторно используем массив дескрипторов
					// и засыпаем, чтобы потоки успели отработать
					count = 0;
					sleep(3);
				}				
			}										
		}						
	}

	// завершение работы
	EXIT:

	stop_threads = 1;	

	// ожидание завершения клиентских потоков
	for (int i = 0; i < start_alloc_size; i++) {
		if (pthread_join(threads_clients[i], NULL) != 0) {
			printf(" pthread_join threads_clients[%d] failed \n", i);						
		}
	}

	// ожидание завершение потока подсчета клиентов
	if (pthread_join(thread_count_connected_clients, NULL) != 0) {
		printf(" pthread_join thread_count_connected_clients failed \n");					
	}

	// ожидание завершения потока управления вводом
	if (pthread_join(thread_input, NULL) != 0) {
		printf(" pthread_join thread_input failed \n");					
	}
	
	for (int i = 0; i < start_alloc_size; i++) {
		if (pthread_mutex_destroy(&ml_thread[i]) != 0) {
			printf(" pthread_mutex_destroy failed \n");				
		}
	}

	// закрываем очередь
	if (mq_close(que) == -1) {
		perror(" сlosing queues failed ");				
	}
	// удаляем очередь
	if (mq_unlink(QUE_NAME) == -1) {
		perror(" removing queues failed ");			
	}

	// проверяем, все ли клиентские сокеты закрыты
	for (int i = 0; i < start_alloc_size; i++) {
		if (clients_fd[i] != 0) {
			close(clients_fd[i]);
		}
	}

	// освобождение памяти
	free(ml_thread);
	free(args);
	free(threads_clients);
	free(addr_clients_size);
	free(addr_clients);
	free(clients_fd);

	close(server_fd);
	exit(EXIT_SUCCESS);
}

void* func_thread_client (void* arg) {

	// принимаем аргумент-структуру
	struct pthread_args* thread_arg = (struct pthread_args*) arg;

	// для mq_receive()
	int res_mqrecv, prio;
	char msg_from_que_buff [BUFF_SIZE];
	memset(msg_from_que_buff, 0, sizeof(msg_from_que_buff));
	int cli_fd_no_from_que;

	// для обмена сообщениями с клиентом
	char thread_buff_input [BUFF_SIZE];
	char thread_buff_output [BUFF_SIZE];
	memset(thread_buff_output, 0, sizeof(thread_buff_output));
	char thread_buff_no [BUFF_SIZE];

	while (stop_threads != 1) {
		// проверка очереди		
		res_mqrecv = mq_receive(que, msg_from_que_buff, sizeof(msg_from_que_buff), &prio);
		
		// если вылетает EAGAIN на неблокирующей очереди, 
		// то уходим на следующую итерацию while'a 
		if (res_mqrecv == -1 && errno == 11) {
			continue;
		}
		// любая другая ошибка
		else if (res_mqrecv == -1){
			printf("mq_receive failed in %d thread\n", thread_arg->thread_no);
			continue;
		}
		// успех вызова mq_receive()
		else {
			// получаем номер клиентского сокета для работы
			cli_fd_no_from_que = atoi(msg_from_que_buff);

			// заполняем свою структуру 
			pthread_mutex_lock(&ml_thread[thread_arg->thread_no]);
			thread_arg->client_fd = clients_fd[cli_fd_no_from_que];
			thread_arg->flag_connect = 1;
			pthread_mutex_unlock(&ml_thread[thread_arg->thread_no]);
		}		 
		
		// работаем с полученным сокетом
		if (thread_arg->flag_connect == 1) {

			strcpy(thread_buff_output, SERVER_MSG);
			sprintf(thread_buff_no, "%d", thread_arg->thread_no);
			strcat(thread_buff_output, thread_buff_no);

			// отправляем/принимаем данные	
			if (recv(thread_arg->client_fd, &thread_buff_input, sizeof(thread_buff_input), 0) == -1) {
				printf("recv() thread[%d] error, errno %d\n", thread_arg->thread_no, errno);											
			}
			else {
				printf("thread [%d] recv msg:%s\n", thread_arg->thread_no, thread_buff_input);
			}
			if (send(thread_arg->client_fd, thread_buff_output, sizeof(thread_buff_output), 0) == -1) {
				printf("send() thread[%d] error, errno %d\n", thread_arg->thread_no, errno);								
			}

			// в конце работы с клиентом
			// закрываем клиентский сокет и зануляем переменные в массивах
			close(clients_fd[cli_fd_no_from_que]);
			clients_fd[cli_fd_no_from_que] = 0;
			memset(&addr_clients[cli_fd_no_from_que], 0, sizeof(struct sockaddr_in));

			// перезаполняем свою структуру
			pthread_mutex_lock(&ml_thread[thread_arg->thread_no]);
			thread_arg->client_fd = 0;
			thread_arg->flag_connect = 0;
			pthread_mutex_unlock(&ml_thread[thread_arg->thread_no]);

			memset(msg_from_que_buff, 0, sizeof(msg_from_que_buff));
			memset(thread_buff_input, 0, sizeof(thread_buff_input));
			memset(thread_buff_output, 0, sizeof(thread_buff_output));
			memset(thread_buff_no, 0, sizeof(thread_buff_no));			
		}		
	} 
	pthread_exit(NULL);
}

void* func_thread_input_server (void* arg) {
	
	while(stop_threads != 1) {

		fgets(buff_input_keyboard, BUFF_SIZE, stdin);
		printf("server input from keyboard:%s\n", buff_input_keyboard);
		if (!strncmp(buff_input_keyboard, EXIT_BUFF_MSG, EXIT_BUFF_SIZE)) {
			main_while_var = 0;			
			break;
		}
		else {
			memcpy(buff_input_for_work, buff_input_keyboard, sizeof(buff_input_keyboard));
			memset(buff_input_keyboard, 0, sizeof(buff_input_keyboard));
		}
	}
	pthread_exit(NULL);
}

void* func_thread_count_connected_clients (void* arg) {

	while (stop_threads != 1) {
		
		// считаем количество подключенных клиентов
		clients_connected_count = 0;
		for (int i = 0; i < start_alloc_size; i++) {
			pthread_mutex_lock(&ml_thread[i]);
			if (args[i].flag_connect == 1) {
				clients_connected_count++;
			}
			pthread_mutex_unlock(&ml_thread[i]);
		}
		printf("'%d' clients connected\n", clients_connected_count);

		sleep(1);
	}
	pthread_exit(NULL);
}