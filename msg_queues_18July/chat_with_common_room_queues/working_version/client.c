#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// начальное имя очереди сообщений
#define START_QUE_NAME "/myqueue"

#define ID_CLIENT_SIZE 32

#define NAME_QUE_BUFF_SIZE 16
#define BUFF_CONNECT_SIZE 16

// приоритет служебных сообщений
#define PRIO_SERVICE_MSG 1

// приоритет обычных сообщений
#define PRIO_REGULAR_MSG 2

// размер сообщения
#define Q_SIZE 256

// переменная очередей
mqd_t que_out, que_in;

// переменные потоков
pthread_t thread_send;
pthread_t thread_receive;
pthread_t thread_scanf;

// переменная для принудительного завершения потоков
int stop_threads = 0;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int err_flag = 0;

// функция потока-отправителя
void* func_thread_send (void* arg);

// функция потока-получателя
void* func_thread_receive (void* arg);

// функция потока ввода с клавиатуры
void* func_thread_scanf (void* arg);

// обработчик CTRL + C
void my_handler_CTRL_C (int sig);

// переменная условия главного цикла
int main_while_var = 1;

// для идентификации клиента в чате
char id_client[ID_CLIENT_SIZE];

// для ввода с клавиатуры
char msg_scanf [Q_SIZE];
int scanf_flag = 0;

// argv[1]- номер для отпределения очередей
// argv[2]- id клиента (Name: )
int main (int argc, char** argv) {

	// очищаем терминал
	system("clear");

	// получаем ID клиента
	strcpy(id_client, argv[2]);

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);

	if (argc != 3) {
		printf(" Invalid count of arguments\n");
		exit(EXIT_FAILURE);
	}

	/* через argv[1] получаем номер, который нужно добавить к 
	   статовому имени очереди, чтобы получить имя очереди для коннекта. 
	   Каждый клиент получает две очереди, номера которых высчитываются:
	   num_que_out = argv[1] * 2; int num_que_in = argv[1] * 2 + 1 */
	char name_queue[NAME_QUE_BUFF_SIZE];
	int num_que_out = atoi(argv[1]) * 2;
	char nu_que_out [2];
	sprintf(nu_que_out, "%d", num_que_out);	
	strcpy(name_queue, START_QUE_NAME);
	strcat(name_queue, nu_que_out);
	
	// открытие очереди на отправку с проверкой
	if ((que_out = mq_open(name_queue, O_RDWR|O_NONBLOCK , 0777, NULL)) == (mqd_t)-1) {
    	perror(" opening queue failed ");
		exit(EXIT_FAILURE);
	}

	// получение имени для второй очереди
	int num_que_in = atoi(argv[1]) * 2 + 1;
	memset(name_queue, 0, NAME_QUE_BUFF_SIZE);
	strcpy(name_queue, START_QUE_NAME);
	char nu_que_in [2];
	sprintf(nu_que_in, "%d", num_que_in);
	strcat(name_queue, nu_que_in);

	// открытие очереди на прием с проверкой
	if ((que_in = mq_open(name_queue, O_RDWR|O_NONBLOCK , 0777, NULL)) == (mqd_t)-1) {
    	perror(" opening queue failed ");
		exit(EXIT_FAILURE);
	}

	// подключение к серверу
	char msg_buff_connect[BUFF_CONNECT_SIZE];
	sprintf(msg_buff_connect, "%d", getpid());
	
	if(mq_send(que_out, msg_buff_connect, strlen(msg_buff_connect),\
	PRIO_SERVICE_MSG) == -1) {
		printf(" mq_send connect msg failed");
		err_flag++;
		goto END;
	}	

	// создание потоков
	if (pthread_create(&thread_send, NULL, func_thread_send, NULL) != 0) {
			printf(" Create thread_send failed");
			err_flag++;
			goto END;
	}
	if (pthread_create(&thread_receive, NULL, func_thread_receive, NULL) != 0) {
			printf(" Create thread_receive failed");
			err_flag++;
			goto END;
	}
	if (pthread_create(&thread_scanf, NULL, func_thread_scanf, NULL) != 0) {
			printf(" Create thread_scanf failed");
			err_flag++;
			goto END;
	}	

	// главный цикл (завершение работы клиента нажатием CTRL + C)
	while(main_while_var) {

		sleep(1);
	}
	
	// отключение от сервера	
	if(mq_send(que_out, msg_buff_connect, strlen(msg_buff_connect),\
	PRIO_SERVICE_MSG) == -1) {
		printf(" mq_send disconnect msg failed");
	}
	

	// завершение и высвобождение ресурсов
	END:
	stop_threads = 1;
	
	// дожидаемся завершения потоков
	if (pthread_join(thread_send, NULL) != 0) {
		printf(" join thread_send failed");
		err_flag++;
	}
	if (pthread_join(thread_receive, NULL) != 0) {
		printf(" join thread_receive failed");
		err_flag++;
	}
	pthread_kill(thread_scanf, SIGTERM);

	// закрываем очереди с проверкой
	if (mq_close(que_out) == -1) {
		perror(" сlosing queue failed ");	
	}
	// закрываем очереди с проверкой
	if (mq_close(que_in) == -1) {
		perror(" сlosing queue failed ");	
	}

	if (err_flag == 0) {
		exit(EXIT_SUCCESS);
	}
	else {
		exit(EXIT_FAILURE);
	}
}

// для потока ввода, отправляет сообщения в очередь
void* func_thread_send (void* arg) {

	char msg_buff_send [Q_SIZE];	
	int mq_send_res = 0;

	while (stop_threads != 1) {

		if (scanf_flag == 1) {

			if (mq_send_res != -1) {
			
				strcpy(msg_buff_send, id_client);
				strcat(msg_buff_send, msg_scanf);
			}			

			if ((mq_send_res = mq_send(que_out, msg_buff_send, strlen(msg_buff_send),\
				PRIO_REGULAR_MSG)) == -1) {
				perror(" sending msg service failed ");
   			}
   				else {
   					memset(msg_buff_send, 0, Q_SIZE);
   					memset(msg_scanf, 0, Q_SIZE);
   					scanf_flag = 0;   					
   				}			
		}	
		sleep(1);
	}
	pthread_exit(NULL);
}

// для потока ввода с клавиатуры, наполняет буфер на отправку
void* func_thread_scanf (void* arg) {

	while (stop_threads != 1) {
		
		if (scanf_flag == 0) {
			scanf("%s", msg_scanf);
			scanf_flag = 1;
		}
	}	
}

// для потока приема
void* func_thread_receive (void* arg) {	

	char msg_buff_receive [Q_SIZE];
	int prio;

	while (stop_threads != 1) {
		
		// принимаем сообщение из очереди		
		if (mq_receive(que_in, msg_buff_receive, Q_SIZE, \
			&prio) == -1) {
			continue;			
		}
			else {
				printf("%s\n", msg_buff_receive);
				memset(msg_buff_receive, 0, Q_SIZE);
			}
		
		sleep(1);
	}
	pthread_exit(NULL);
}

// обработчик CTRL + C - меняет условие в главном цикле 
// главного потока
void my_handler_CTRL_C (int sig) {
	main_while_var = 0;
}