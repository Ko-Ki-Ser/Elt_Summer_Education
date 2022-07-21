#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>

// размер сообщения
#define MSG_BUFF_SIZE 128

// размер участка памяти
#define SHMEM_SIZE 1024

// максимальный размер никнейма
#define ID_CLIENT_SIZE 32

// сообщение для подключения
#define MSG_CONNECT "connect"

// сообщение для отключения
#define MSG_DISCONNECT "disconnect"

// максимальный размер имен
#define NAME_SHMEM_SEM_SIZE 16

// начальное имя участков разделяемой памяти
#define START_SHMEM_NAME "/myshmem"

// начальное имя семафоров
#define START_SEMAPHORE_NAME "/mysem"

// объявление структуры для взаимодействия клиентов
// с общей памятью
typedef struct str_msg_from_or_to_client {
	char msg_buff [MSG_BUFF_SIZE];
	int flag;
}struct_msg_client;

// буферы для входящих и исходящих сообщений
struct_msg_client* msg_out;
struct_msg_client* msg_in;

// переменные потоков
pthread_t thread_send;
pthread_t thread_receive;
pthread_t thread_scanf;

// переменная для принудительного завершения потоков
int stop_threads = 0;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int err_flag = 0;

// переменная условия главного цикла
int main_while_var = 1;

// для идентификации клиента в чате
char id_client[ID_CLIENT_SIZE];

// для ввода с клавиатуры
char msg_scanf [MSG_BUFF_SIZE];
int scanf_flag = 0;

// номера для участков памяти
int num_in, num_out;

// имена участков памяти
char shmem_name_in[NAME_SHMEM_SEM_SIZE];
char shmem_name_out[NAME_SHMEM_SEM_SIZE];

// дескрипторы участков памяти
int shmem_d_in;
int shmem_d_out;

// указатели на участки памяти
void* shmem_ptr_in;
void* shmem_ptr_out;

// имена семафоров
char sem_name_in[NAME_SHMEM_SEM_SIZE];
char sem_name_out[NAME_SHMEM_SEM_SIZE];

// переменные семафоров
sem_t* sem_in;
sem_t* sem_out;

// функция потока-отправителя
void* func_thread_send (void* arg);

// функция потока-получателя
void* func_thread_receive (void* arg);

// функция потока ввода с клавиатуры
void* func_thread_scanf (void* arg);

// обработчик CTRL + C
void my_handler_CTRL_C (int sig);

// argv[1]- номер для определения участков памяти
// argv[2]- id клиента (Name: )
int main (int argc, char** argv) {
	
	if (argc != 3) {
		printf(" Invalid count of arguments\n");
		exit(EXIT_FAILURE);
	}

	// очищаем терминал
	system("clear");

	// делаем свой обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);

	//инициализация буферов
	msg_out = calloc (1, sizeof(struct_msg_client));
	msg_in = calloc (1, sizeof(struct_msg_client));

	// получаем номера
	// считаем номер для участка памяти на прием
	num_in = (atoi(argv[1])) * 2 + 1;

	// считаем номер для участка памяти на отправку
	num_out = (atoi(argv[1])) * 2;	

	// получаем ID клиента
	strcpy(id_client, argv[2]);
	
	// временные буферы для номеров
	char buf_num_in[2];
	char buf_num_out[2];

	// получение имен участков памяти и имен семафоров
	sprintf(buf_num_in, "%d", num_in);
	sprintf(buf_num_out, "%d", num_out);

	strcpy(shmem_name_in, START_SHMEM_NAME);
	strcpy(shmem_name_out, START_SHMEM_NAME);
	strcpy(sem_name_in, START_SEMAPHORE_NAME);
	strcpy(sem_name_out, START_SEMAPHORE_NAME);

	strcat(shmem_name_in, buf_num_in);
	strcat(shmem_name_out, buf_num_out);
	strcat(sem_name_in, buf_num_in);
	strcat(sem_name_out, buf_num_out);

	// открываем семафоры
	if ((sem_in = sem_open (sem_name_in, O_CREAT, 0777, 1)) == SEM_FAILED) {
			perror(" sem_in_open error ");
			err_flag++;
			goto END;
	}

	if ((sem_out = sem_open (sem_name_out, O_CREAT, 0777, 1)) == SEM_FAILED) {
			perror(" sem_out_open error ");
			err_flag++;
			goto END;
		}

	// открываем два участка памяти
	if ((shmem_d_in = shm_open(shmem_name_in, O_CREAT|O_RDWR, 0777)) == -1) {
    		perror(" shm_in_open error ");
    		err_flag++;
			goto END;
  	}
  	if ((shmem_d_out = shm_open(shmem_name_out, O_CREAT|O_RDWR, 0777)) == -1) {
    		perror(" shm_out_open error ");
    		err_flag++;
			goto END;
  	}

  	// устанавливаем размер
  	if (ftruncate(shmem_d_in, SHMEM_SIZE) != 0) {
    		perror("ftruncate shmem_d_in error");
    		err_flag++;
			goto END;
    }
	if (ftruncate(shmem_d_out, SHMEM_SIZE) != 0) {
    		perror("ftruncate shmem_d_out error");
    		err_flag++;
			goto END;
    }

    // подключаем память в адресное пространство 
    if ((shmem_ptr_in = mmap(0, SHMEM_SIZE, PROT_WRITE|PROT_READ,\
    						 MAP_SHARED, shmem_d_in, 0)) == MAP_FAILED) {
    		perror("mmap shmem_in error");
    		err_flag++;
			goto END;
    }
    if ((shmem_ptr_out = mmap(0, SHMEM_SIZE, PROT_WRITE|PROT_READ,\
    						 MAP_SHARED, shmem_d_out, 0)) == MAP_FAILED) {
    		perror("mmap shmem_out error");
    		err_flag++;
			goto END;
    } 

    // блокируем память
    if (mlock(shmem_ptr_in, SHMEM_SIZE) != 0) {
    	perror("mlock shmem error");
    	err_flag++;
		goto END;    		
    }
    if (mlock(shmem_ptr_out, SHMEM_SIZE) != 0) {
    	perror("mlock shmem error");
    	err_flag++;
		goto END;    		
    }    

    // создание потоков
    if (pthread_create(&thread_send, NULL, func_thread_send, NULL) != 0) {
			printf(" Create thread_sends failed");
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

	// временный буфер
	char* msg_con_discon = calloc(1, MSG_BUFF_SIZE);
	sprintf(msg_con_discon, "%s", MSG_CONNECT);

	// подключение к серверу
	

	// критическая секция
	sem_wait(sem_out);
	memcpy(msg_out->msg_buff, msg_con_discon, strlen(msg_con_discon));
	msg_out->flag = getpid(); 
	memcpy(shmem_ptr_out, msg_out, sizeof(struct_msg_client));		
	sem_post(sem_out);
		

	// главный цикл (завершение работы клиента нажатием CTRL + C)
	while(main_while_var) {

		sleep(1);
	}

	// отключение от сервера
	memset(msg_con_discon, 0, MSG_BUFF_SIZE);
	sprintf(msg_con_discon, "%s", MSG_DISCONNECT);	

	// критическая секция
	sem_wait(sem_out); 
	memcpy(msg_out->msg_buff, msg_con_discon, strlen(msg_con_discon));
	memcpy(shmem_ptr_out, msg_out, sizeof(struct_msg_client));		
	sem_post(sem_out);

	// завершение и высвобождение ресурсов
	END:
	stop_threads = 1;		

	// ожидание завершения потоков
	if (pthread_join(thread_send, NULL) != 0) {
		printf(" pthread_join failed ");
		err_flag++;			
	}
	if (pthread_join(thread_receive, NULL) != 0) {
		printf(" pthread_join failed ");
		err_flag++;			
	}
	if (pthread_join(thread_scanf, NULL) != 0) {
		printf(" pthread_join failed ");
		err_flag++;			
	}
	//pthread_kill(thread_scanf, SIGTERM);

	// отделяем память от адресного пространства
	munmap(shmem_ptr_in, SHMEM_SIZE);
	munmap(shmem_ptr_out, SHMEM_SIZE);

	// закрываем дескрипторы
	close(shmem_d_in);
	close(shmem_d_out);

	// закрываем семафоры
	sem_close(sem_in);
	sem_close(sem_out);

	free(msg_con_discon);
	free(msg_in);
	free(msg_out);

	if (err_flag == 0) {
		exit(EXIT_SUCCESS);
	}
	else {
		exit(EXIT_FAILURE);
	}
}

// отправляет сообщения в память
void* func_thread_send (void* arg) {

	char msg[MSG_BUFF_SIZE] = {0};

	while(stop_threads != 1) {
		if (scanf_flag == 1) {
			// критическая секция
			if (sem_trywait(sem_out) == 0) {

				strcpy(msg, id_client);
				strcat(msg, msg_scanf);

				memcpy(msg_out->msg_buff, msg, strlen(msg_scanf));
				msg_out->flag = 1;
				memcpy(shmem_ptr_out, msg_out, sizeof(struct_msg_client));
				scanf_flag = 0;
				memset(msg_out, 0, sizeof(struct_msg_client));
				memset(msg, 0, sizeof(msg));

				sem_post(sem_out);
			}
		}
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
	pthread_exit(NULL);	
}

// для потока приема
void* func_thread_receive (void* arg) {

	while(stop_threads != 1) {
		if (sem_trywait(sem_in) == 0) {
			memcpy(msg_in, shmem_ptr_in, sizeof(struct_msg_client));
			if (msg_in->flag == 1) {
				printf("%s\n", msg_in->msg_buff);
				msg_in->flag = 0;
				memcpy(shmem_ptr_in, msg_in, sizeof(struct_msg_client));
			}
			sem_post(sem_in);
		}
	}
	pthread_exit(NULL);
}

// обработчик CTRL + C - меняет условие в главном цикле 
// главного потока
void my_handler_CTRL_C (int sig) {
	main_while_var = 0;
}