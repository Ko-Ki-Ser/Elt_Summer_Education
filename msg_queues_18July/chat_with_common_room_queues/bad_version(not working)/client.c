#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>


// имя очереди сообщений
#define QUE_NAME "/myqueue"

// приоритет служебных сообщений
#define PRIO_SERVICE_MSG 1

// приоритет обычных сообщений
#define PRIO_REGULAR_MSG 3

// приоритет для рассылки сообщений
#define PRIO_SEND_MSG 2

// размер сообщения
#define Q_SIZE 256

// функция для потока приемщика
void* func_receive (void* arg);

mqd_t que;

int choice;

// переменные для потока приемщика
char msg_to_send [Q_SIZE];
char buff_itoa [Q_SIZE];
char msg_to_receive [Q_SIZE];
int priority = 0;
int count = 0;
int stop_thread = 0;

// переменные для отправки
char send_buff [Q_SIZE];
int send_priority;


int main (void) {

	// открытие очереди с проверкой
	if ((que = mq_open(QUE_NAME, O_RDWR , 0777, NULL)) == (mqd_t)-1) {
    	perror(" opening queue failed ");
		exit(EXIT_FAILURE);
	}

	// id потока
	pthread_t thread_receive;
	
	if (pthread_create(&thread_receive, NULL, func_receive,\
						NULL) != 0) {
		printf(" Create thread_receive failed");
		exit(EXIT_FAILURE);
	}

	sprintf(send_buff, "%d", getpid());
	send_priority = 1;

	// подключаемся к серверу
	if (mq_send(que, send_buff, strlen(send_buff), send_priority) == -1) {
		perror(" sending main msg failed ");
   	}

   	memset(send_buff, 0, Q_SIZE);
   	send_priority = 0;

	// основной цикл работы программы клиента
	while (1) {

		printf("Do you want send message [1] or close client [2]?\n");
		scanf("%d", &choice);

		if (choice == 1) {

			printf("Enter message\n");
			scanf("%s", send_buff);
			printf("Enter priority\n");
			scanf("%d", &send_priority);

			if (mq_send(que, send_buff,\
				strlen(send_buff), send_priority)\
				== -1) {
				perror(" sending main msg failed ");
   			}

   		}

		else if (choice == 2) {
			stop_thread = 1;
			break;
		}

	}

	/* корректное завершение работы клиента*/
	// ожидаем завершения потоков
	if (pthread_join(thread_receive, NULL) != 0) {
		printf(" join thread_receive failed");
		exit(EXIT_FAILURE);
	}

	// закрываем очередь с проверкой
	if (mq_close(que) == -1) {
		perror(" сlosing queue failed ");	
	}

	exit(EXIT_SUCCESS);
}

void* func_receive (void* arg) {

	while (1) {

		// принимаем сообщение из очереди
		if (mq_receive(que, msg_to_receive, Q_SIZE, \
			&priority) == -1) {
			perror(" receiving msg failed ");
			// если ошибка, перепрыгиваем след. проверку
			goto END_CLIENT;
		}

		/* у полученного сообщения отделяем счетчик уменьшаем его на 1
		и кладем сообщение обратно*/
		if (priority == PRIO_SEND_MSG) {
				
			for (int i = 0; i < Q_SIZE; i++) {
				if (msg_to_receive[i] == '\0') {
					count = atoi ((char)msg_to_receive[i-1]);
					count--;
					memcpy (msg_to_send, msg_to_receive, i-2);
					break;
				} 
			}
			// печатаем сообщение
			printf("\n%s\n", msg_to_send);
			/* преобразуем число оставшихся клиентов в строку и прилепляем 
			ее к исходному сообщению */
			sprintf(buff_itoa,"%d", count);
			strcat(msg_to_send, buff_itoa);
			// кладем новое творение обратно в очередь
			if (mq_send(que, msg_to_send,\
				strlen(msg_to_send), PRIO_SEND_MSG)\
				== -1) {
				perror(" sending msg reg failed ");
   			}
		}
		// если служебное, то кладем его обратно в очередь
		else  if (priority == PRIO_SERVICE_MSG) {
			if (mq_send(que, msg_to_receive,\
				strlen(msg_to_receive), priority)\
				== -1) {
				perror(" sending msg service failed ");
   			}
		}
		END_CLIENT:
		/* обнуляем переменные */
		memset(msg_to_send, 0, Q_SIZE);
		memset(msg_to_receive, 0, Q_SIZE);
		memset(buff_itoa, 0, Q_SIZE);
		count = 0;
		priority = 0;

		// условие завершения потока
		if (stop_thread == 1) {
			break;
		}
		sleep(1);
	}

	pthread_exit(NULL);
}
