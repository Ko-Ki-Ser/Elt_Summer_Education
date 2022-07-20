/* чат с общей комнатой с использованием очередей сообщений */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>

// начальное имя очереди сообщений
#define START_QUE_NAME "/myqueue"

#define NAME_QUE_SIZE 16

// максимальное число клиентов
#define MAX_CLIENTS 3

// приоритет служебных сообщений
#define PRIO_SERVICE_MSG 1

// приоритет обычных сообщений
#define PRIO_REGULAR_MSG 2

// размер сообщения
#define Q_SIZE 256

// при создании очереди сообщений с mq_attr.mq_maxmsg > 10
// вылетает ошибка EINVAL (Invalid argument) (errno 22)
#define MAX_MSG 10

// структура для хранения данных о клиентах
struct clients_info {
	// pid процесса клиента
	pid_t client_pid; 
	// флажок подключен/отключен
	int flag_connect;
};

// мьютексы для синхронизации потоков
pthread_mutex_t* mutex_ques;

// переменная для принудительного завершения потоков
int stop_threads = 0;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int err_flag = 0;

// для параметров очередей
struct mq_attr* ques_attr;

// имена очередей
char** ques_names;

// переменные очередей
mqd_t* ques;

// инфа о клиентах
struct clients_info* clients_info;

// переменные потоков
pthread_t* threads_for_clients;

// количество подключенных клиентов
int clients_connected_count = 0;

// функция для клиентских потоков
void* func_thread_client (void* arg);

// обработчик CTRL + C
void my_handler_CTRL_C (int sig);

int main_while_var = 1;

int main (void) {

	// очищаем терминал
	system("clear");

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);

	/* создание массива структур 
	для параметров очереди */
	ques_attr = calloc (MAX_CLIENTS * 2, sizeof(struct mq_attr));
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		// максимальные число сообщений и размер очереди для каждой очереди
		ques_attr[i].mq_maxmsg = MAX_MSG;
		ques_attr[i].mq_msgsize = Q_SIZE;
	}
	
	// временный буфер для номера
	char name_no_buff[2];

	// выделение массива под имена очередей сообщений
	ques_names = (char**)malloc(sizeof(char*) * MAX_CLIENTS * 2);
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		ques_names[i] = (char*)calloc(1, NAME_QUE_SIZE * sizeof(char));
		
		// начальное имя + номер
		strcpy(ques_names[i], START_QUE_NAME);
		sprintf(name_no_buff, "%d", i);
		strcat(ques_names[i], name_no_buff);
		memset(name_no_buff, 0, strlen(name_no_buff));		
	}
	
	/* создание массива переменных 
	для очередей сообщений (по две очереди на каждого клиента)*/
	ques = malloc (sizeof(mqd_t) * MAX_CLIENTS * 2);

	// создание очередей сообщений (по две на каждого клиента)
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		if ((ques[i] = mq_open(ques_names[i], O_CREAT|O_RDWR|O_NONBLOCK , 0777,\
					   &ques_attr[i])) == (mqd_t)-1) {
    	perror(" creating queue failed ");
		err_flag++;
		goto END;
		}

	}

	// выделение памяти под мьютексы
	mutex_ques = malloc (MAX_CLIENTS * 2 * sizeof(pthread_mutex_t));
	 
	// инициализация мьютексов для работы с очередями сообщений
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		if (pthread_mutex_init(&mutex_ques[i], NULL) != 0) {
			printf(" pthread_mutex_init failed");
			err_flag++;
			goto END;	
		}
	}

	/* создание массива данных о клиентах*/
	clients_info = calloc (MAX_CLIENTS, sizeof(struct clients_info));
	/* создание потоков */
	// по одному на каждого клиента
	threads_for_clients = calloc(MAX_CLIENTS, sizeof(pthread_t));

	// для передачи аргументов в потоки
	int pthread_args [MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++) {
		pthread_args[i] = i;		
	}

	// создание потоков
	for (int i = 0; i < MAX_CLIENTS; i++) {	
		if (pthread_create(&threads_for_clients[i], NULL, func_thread_client,\
						  (void*) &pthread_args[i]) != 0) {
			printf(" Create threads failed");
			err_flag++;
			goto END;
		}
	}

	// главный цикл работы сервера (завершение нажатием CTRL + C)
	while(main_while_var) {
				
		printf("Server: clients connected: %d\n", clients_connected_count);
		for (int i = 0; i < MAX_CLIENTS; i++) {
			printf("clients_info[%d].client_pid:%d\n", i, clients_info[i].client_pid);
		}
		
		sleep(20);
	}

	// освобождение всех ресурсов и завершение программы
	END:

	// ожидание завершения потоков
	stop_threads = 1;
	for (int i = 0; i < MAX_CLIENTS; i++) {	
		if (pthread_join(threads_for_clients[i], NULL) != 0) {
			printf(" pthread_join failed ");
			err_flag++;			
		}
	}

	// уничтожение мьютексов
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		if (pthread_mutex_destroy(&mutex_ques[i]) != 0) {
			printf(" pthread_mutex_destroy failed ");
			err_flag++;
		}
	}

	// закрываем очереди
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		if (mq_close(ques[i]) == -1) {
			perror(" сlosing queues failed ");
			err_flag++;	
		}
	}

	// удаляем очереди из системы
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		if (mq_unlink(ques_names[i]) == -1) {
			perror(" removing queues failed ");
			err_flag++;
		}
	}

	free(ques_attr);
	free(ques);
	for (int i = 0; i < MAX_CLIENTS * 2; i++) {
		free (ques_names[i]);
	}
	free(ques_names);
	free(clients_info);
	free(threads_for_clients);
	free(mutex_ques);

	if (err_flag == 0) {
		exit (EXIT_SUCCESS);
	}
	else {
		exit(EXIT_FAILURE);
	}
}


void* func_thread_client (void* arg) {

	// принятый аргумент
	int* arg_num = (int*) arg;

	// считаем номер очереди
	int num_que_in = (*arg_num) * 2;	
	
	// буфер для сообщений и приоритета
	char msg_buff [Q_SIZE];
	int prio;

	// для проверки функции mq_receive()
	int mq_res = 0;

	while (stop_threads != 1) {

		if (pthread_mutex_trylock(&mutex_ques[num_que_in]) == 0) {

			mq_res = mq_receive(ques[num_que_in], msg_buff, sizeof(msg_buff), &prio);

			if (mq_res == -1) {
				pthread_mutex_unlock(&mutex_ques[num_que_in]);
				continue;
			}
			pthread_mutex_unlock(&mutex_ques[num_que_in]);
		}	

		// обработка служебного сообщения от своего клиента
		if (prio == PRIO_SERVICE_MSG) {
				
			// клиент подключается
			if (clients_info[*(arg_num)].flag_connect == 0) {
				clients_info[*(arg_num)].client_pid = atoi(msg_buff);
				clients_info[*(arg_num)].flag_connect = 1;
				clients_connected_count++;
			}
				// клиент отключается
				else if (clients_info[*(arg_num)].flag_connect == 1) {
					clients_info[*(arg_num)].flag_connect = 0;
					clients_info[*(arg_num)].client_pid = 0;
					clients_connected_count--;
				}

		}
			// обработка обычного сообщения от своего клиента
			else if (prio == PRIO_REGULAR_MSG) {
									
				printf("%s\n", msg_buff);
				// в цикле пытаемся захватывать другие блокировки, 
				// чтобы разослать полученное сообщение другим клиентам
				for (int i = 1; i < MAX_CLIENTS * 2; i+=2) {						
						
					if (i != (num_que_in + 1)) {
						pthread_mutex_lock(&mutex_ques[i]); 	
						mq_send(ques[i], msg_buff, strlen(msg_buff), PRIO_REGULAR_MSG);
						pthread_mutex_unlock(&mutex_ques[i]);
					}					
				}												
			}
			
			memset(msg_buff, 0, Q_SIZE);
			mq_res = 0;
			
	}	
	pthread_exit(NULL);
} 


void my_handler_CTRL_C (int sig) {
	main_while_var = 0;
}