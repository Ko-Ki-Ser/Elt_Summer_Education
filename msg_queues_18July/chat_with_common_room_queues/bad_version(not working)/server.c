/* Сервер чата с общей комнатой, использующий очередь сообщений с приоритетом,
семафор и потоки */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <mqueue.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>


// имя очереди сообщений
#define QUE_NAME "/myqueue"

// сообщение для пользователя продолжать работу сервера или нет
#define MSG_TO_USR_CHOICE "\nNo clients connected. Do you want\
continue [1] or stop server [2] ?\n"

// приоритет служебных сообщений
#define PRIO_SERVICE_MSG 1

// приоритет обычных сообщений
#define PRIO_REGULAR_MSG 3

// приоритет для рассылки сообщений
#define PRIO_SEND_MSG 2

// максимальное число клиентов чата
#define MAX_CLIENTS 3

// размер сообщения
#define Q_SIZE 256

// при создании очереди сообщений с mq_attr.mq_maxmsg > 10
// вылетает ошибка EINVAL (Invalid argument) (errno 22)
#define MAX_MSG 10

// функция потока, ожидающего обычные сообщения
void* func_wait_reg_msg (void* arg);

// функция потока, ожидающего служебные сообщения
void* func_wait_service_msg (void* arg);

// обработчик CTRL + C
void my_handler_CTRL_C (int sig);

/* сущности для очереди сообщений */
// тип возвращаемый при создании очереди сообщений
mqd_t que;
	
// структура для параметров очереди
struct mq_attr que_attr;

// мьютекс для синхронизации потоков
pthread_mutex_t mutex_que;

// переменная для принудительного завершения потоков
int stop_threads = 0;

// флаг для фиксации первого подключения к серверу
int the_first_connect_flag = 0;

// структура для хранения данных о клиентах
struct clients_info {
	// pid процесса клиента
	pid_t client_pid; 
	// флажок подключен/отключен
	int flag_connect;
};

// массив этих структур
struct clients_info clients[MAX_CLIENTS] = {0};

// буферы потоков для сообщения
char msg_buff_thread_reg[Q_SIZE];
char msg_buff_thread_service[Q_SIZE];

// переменные потоков для приоритета
int priority_thread_reg;
int priority_thread_service;

// кол-во подключенных клиентов для func_wait_reg_msg
int count_of_con_client;

// массив для itoa (используется в func_wait_reg_msg)
char buff_itoa[Q_SIZE];

int main (void) {

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);

	// максимальные число сообщений и размер очереди
	que_attr.mq_maxmsg = MAX_MSG;
	que_attr.mq_msgsize = Q_SIZE;

	// инициализация мьютекса для работы с очередью сообщений
	if (pthread_mutex_init(&mutex_que, NULL) != 0) {
		printf(" pthread_mutex_init failed");
		exit(EXIT_FAILURE);
	}

	// создание очереди с проверкой на ошибку
	if ((que = mq_open(QUE_NAME, O_CREAT|O_RDWR , 0777, &que_attr)) == (mqd_t)-1) {
    	perror(" creating queue failed ");
		exit(EXIT_FAILURE);
	}
	
	/* создание потоков */
	// id потоков
	pthread_t thread_wait_reg_msg;
	pthread_t thread_wait_service_msg;
	
	if (pthread_create(&thread_wait_reg_msg, NULL, func_wait_reg_msg, NULL) != 0) {
		printf(" Create thread_wait_reg_msg failed");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&thread_wait_service_msg, NULL, func_wait_service_msg,\
						NULL) != 0) {
		printf(" Create thread_wait_service_msg failed");
		exit(EXIT_FAILURE);
	}
		
	/* основной цикл работы сервера 
	проверяется содержимое переменной the_first_connect_flag, если было первое
	подключение, то проверяем в бесконечном цикле int flag_connect в массиве
	структур, когда нет ни одного подключенного клиента, спрашиваем пользователя, 
	завершить или продолжить работу сервера*/
	int usr_choice = 0;

	while (1) {

		if (the_first_connect_flag == 1) {
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].flag_connect == 1) {
					break;
				}
				/* если дошли до последней структуры при the_first_connect_flag = 1
				   и нам не встретилось ни одного flag_connect = 1, то спрашиваем 
				   пользователя */
				if (i == MAX_CLIENTS - 1) {
					printf(MSG_TO_USR_CHOICE);
					scanf("%1d", &usr_choice);
					// выход
					if (usr_choice == 2) {
						the_first_connect_flag = 0;
						stop_threads = 1;
						goto End;
					}
					// продолжаем
					else if (usr_choice == 1) {
						the_first_connect_flag = 0;
						break;
					}
				}
			}
		}
		// чтоб не занимал много процессорного времени
		sleep(1);
	}

	End:
	/* корректное завершение работы сервера*/
	// ожидаем завершения потоков
	if (pthread_join(thread_wait_reg_msg, NULL) != 0) {
		printf(" join thread_wait_reg_msg failed");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_join(thread_wait_service_msg, NULL) != 0) {
		printf(" join thread_wait_service_msg failed");
		exit(EXIT_FAILURE);
	}
	
	// уничтожение мьютекса
	if (pthread_mutex_destroy(&mutex_que) != 0) {
		printf(" pthread_mutex_init failed");
		exit(EXIT_FAILURE);
	}

	// закрываем очередь
	if (mq_close(que) == -1) {
		perror(" сlosing queue failed ");
		exit(EXIT_FAILURE);	
	}

	// удаляем очередь из системы
	if (mq_unlink(QUE_NAME) == -1) {
		perror(" removing queue failed ");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

// функция потока, ожидающего обычные сообщения
void* func_wait_reg_msg (void* arg) {

	while (1) {

		// работа с очередью сообщений через мьютекс
		if (pthread_mutex_trylock(&mutex_que) == 0) {

			// принимаем сообщение из очереди
			if (mq_receive(que, msg_buff_thread_reg, Q_SIZE, \
				&priority_thread_reg) == -1) {
				perror(" receiving msg failed (service) ");
				// если ошибка, перепрыгиваем след. проверку
				goto MU_UNLK_REG;
			}

			/* если сообщение обычное, печатаем его на экране и кладем
			обратно в очередь с приоритетом PRIO_SEND_MSG с добавлением 
			к концу сообщения служебной группы, указывающей число клиентов, 
			которые должны получить данное сообщение*/
			if (priority_thread_reg == PRIO_REGULAR_MSG) {
				printf("%s\n", msg_buff_thread_reg);
				// подсчет кол-ва клиентов
				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (clients[i].flag_connect == 1) {
						count_of_con_client++;
					}
				}
				/* преобразуем число клиентов в строку и прилепляем 
				ее к исходному сообщению */
				sprintf(buff_itoa,"%d", count_of_con_client);
				strcat(msg_buff_thread_reg, buff_itoa);

				// кладем новое творение обратно в очередь
				if (mq_send(que, msg_buff_thread_reg,\
					strlen(msg_buff_thread_reg), PRIO_SEND_MSG)\
					== -1) {
					perror(" sending msg reg failed ");
    			}
			}
			// если служебное, то кладем его обратно в очередь
			else  if (priority_thread_reg == PRIO_SERVICE_MSG) {
				if (mq_send(que, msg_buff_thread_reg,\
					strlen(msg_buff_thread_reg), priority_thread_reg)\
					== -1) {
					perror(" sending msg reg non-reg failed ");
    			}
			}

			MU_UNLK_REG:
			/* обнуляем буфер и переменную потока для след. итерации, 
			а также счетчик подключенных клиентов и буфер itoa */
			memset(msg_buff_thread_reg, 0, Q_SIZE);
			memset(buff_itoa, 0, Q_SIZE);
			count_of_con_client = 0;
			priority_thread_reg = 0;
			pthread_mutex_unlock(&mutex_que);
		}
		// условие завершения потока
		if (stop_threads == 1) {
			break;
		}
		sleep(1);
	}

	pthread_exit(NULL);
}

// функция потока, ожидающего служебные сообщения
void* func_wait_service_msg (void* arg) {

	while (1) {

		// работа с очередью сообщений через мьютекс
		if (pthread_mutex_trylock(&mutex_que) == 0) {

			// принимаем сообщение из очереди
			if (mq_receive(que, msg_buff_thread_service, Q_SIZE, \
				&priority_thread_service) == -1) {
				perror(" receiving msg failed (service) ");
				// если ошибка, перепрыгиваем след. проверку
				goto MU_UNLK_SER;
			}

			/* если сообщение служебное, заполняем поля свободной 
			   структуры clients_info и the_first_connect_flag, если
			   она равна 0 */
			if (priority_thread_service == PRIO_SERVICE_MSG) {
				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (clients[i].flag_connect == 0){
						clients[i].client_pid = atoi(msg_buff_thread_service);
						clients[i].flag_connect = 1;
						if (the_first_connect_flag == 0) {
							the_first_connect_flag = 1;
						}
					}
				}				
			} 
			// если неслужебное, то кладем его обратно в очередь
			else {
				if (mq_send(que, msg_buff_thread_service,\
					strlen(msg_buff_thread_service), priority_thread_service)\
					== -1) {
					perror(" sending msg failed ");
    			}
			}
			
			MU_UNLK_SER:
			// обнуляем буфер и переменную потока для след. итерации
			memset(msg_buff_thread_service, 0, Q_SIZE);
			priority_thread_service = 0;
			pthread_mutex_unlock(&mutex_que);
		}

		// условие завершения потока
		if (stop_threads == 1) {
			break;
		}
		sleep(1);
	}

	pthread_exit(NULL);
}

// при нажатии CTRL + C завершаем работу сервера без ожидания
// завершения потоков
void my_handler_CTRL_C (int sig) {
	
	// закрываем очередь
	if (mq_close(que) == -1) {
		perror(" сlosing queue failed ");
		exit(EXIT_FAILURE);	
	}

	// удаляем очередь из системы
	if (mq_unlink(QUE_NAME) == -1) {
		perror(" removing queue failed ");
		exit(EXIT_FAILURE);
	}

	stop_threads = 1;

	exit(EXIT_FAILURE);
}