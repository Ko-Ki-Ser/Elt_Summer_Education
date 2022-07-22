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

// размер участка памяти
#define SHMEM_SIZE 1024

// размер сообщения
#define MSG_BUFF_SIZE 128

// максимальный размер имен
#define NAME_SHMEM_SEM_SIZE 16

// сообщение для подключения
#define MSG_CONNECT "connect"
#define CON_COUNT 7

// сообщение для отключения
#define MSG_DISCONNECT "disconnect"
#define DISCON_COUNT 10

// начальное имя участков разделяемой памяти
#define START_SHMEM_NAME "/myshmem"

// начальное имя семафоров
#define START_SEMAPHORE_NAME "/mysem"

// структура для хранения данных о клиентах
struct clients_info {
	// pid процесса клиента
	pid_t client_pid; 
	// флажок подключен/отключен
	int flag_connect;
};

struct clients_info* clients_info;

// объявление структуры для взаимодействия клиентов
// с общей памятью
typedef struct str_msg_from_or_to_client {
	char msg_buff [MSG_BUFF_SIZE];
	int flag;
}struct_msg_client;

// выделение двух буферов для каждого клиента
struct_msg_client** msg_to_cli;
struct_msg_client** msg_from_cli;

// дескрипторы для разделяемой памяти
int* shmem_d;

// указатели для разделяемой памяти
void** shmem_ptr;

// переменные для семафоров
sem_t** sems;

// имена участков памяти
char** shmem_names;

// имена семафоров
char** sem_names;

// переменные потоков для работы
pthread_t* threads_clients;

// для передачи аргументов в потоки
int* pthread_args;

// переменная для принудительного завершения потоков
int stop_threads = 0;

/* переменная для завершения работы (exit(EXIT_FAILURE)
   или exit(EXIT_SUCCESS)) */
int err_flag = 0;

// количество подключенных клиентов
int clients_connected_count = 0;

// функция для клиентских потоков
void* func_thread_client (void* arg);

// обработчик CTRL + C (для завершения работы сервера)
void my_handler_CTRL_C (int sig);

int main_while_var = 1;

int max_clients;

int main (int argc, char** argv) {
	
	if (argc != 2) {
		printf(" Invalid count of arguments\n");
		exit(EXIT_FAILURE);
	}

	// получаем из аргументов кол-во клиентов
	max_clients = atoi(argv[1]);

	// очищаем терминал
	system("clear");

	// обработчик сигнала при нажатии CTRL + C
	signal(SIGINT, my_handler_CTRL_C);

	// выделяем ресурсы
	// выделение памяти: семафоров и участков памяти по два на клиента,
	// остальное всё по одному
	
	// структура инфы о клиентах
	clients_info = calloc(max_clients, sizeof(struct clients_info));
	// дескрипторы разделяемой памяти	
	shmem_d = calloc(max_clients * 2, sizeof(int));
	// буфер для исходящих сообщений (клиенту)
	msg_to_cli = calloc(max_clients, sizeof(void*));
	for (int i = 0; i < max_clients; i++) {
		msg_to_cli[i] = calloc(1, sizeof(struct_msg_client));
	}
	// буфер для входящих сообщений (от клиента)
	msg_from_cli = calloc(max_clients, sizeof(void*));
	for (int i = 0; i < max_clients; i++) {
		msg_from_cli[i] = calloc(1, sizeof(struct_msg_client));
	}
	shmem_ptr = calloc (max_clients * 2, sizeof(void*));
	sems = calloc(max_clients * 2, sizeof(void*));	
	threads_clients = calloc(max_clients, sizeof(pthread_t));
	pthread_args = calloc(max_clients, sizeof(int));
	
	// временные буферы для номеров
	char no_shmem_buff[2];
	char no_sem_buff[2];

	// имена для памяти и семафоров
	shmem_names = (char**)malloc(sizeof(char*) * max_clients * 2);
	sem_names = (char**)malloc(sizeof(char*) * max_clients * 2);
	for (int i = 0; i < max_clients * 2; i++) {
		shmem_names[i] = (char*)calloc(1, NAME_SHMEM_SEM_SIZE * sizeof(char));
		// начальное имя + номер
		strcpy(shmem_names[i], START_SHMEM_NAME);
		sprintf(no_shmem_buff, "%d", i);
		strcat(shmem_names[i], no_shmem_buff);
		memset(no_shmem_buff, 0, strlen(no_shmem_buff));

		sem_names[i] = (char*)calloc(1, NAME_SHMEM_SEM_SIZE * sizeof(char));
		// начальное имя + номер
		strcpy(sem_names[i], START_SEMAPHORE_NAME);
		sprintf(no_sem_buff, "%d", i);
		strcat(sem_names[i], no_sem_buff);
		memset(no_sem_buff, 0, strlen(no_sem_buff));
	}

	// создание семафоров с еденичкой (разблокированы)
	for (int i = 0; i < max_clients * 2; i++) {
		if ((sems[i] = sem_open (sem_names[i], O_CREAT, 0777, 1)) == SEM_FAILED) {
			perror(" sem_open error ");
			err_flag++;
			goto END;
		}		
	}

	// создание разделяемой памяти
	for(int i = 0; i < max_clients * 2; i++) {
		if ((shmem_d[i] = shm_open(shmem_names[i], O_CREAT|O_RDWR, 0777)) == -1) {
    		perror(" shm_open error ");
    		err_flag++;
			goto END;
  		}

  		// устанавливаем размер памяти
  		if (ftruncate(shmem_d[i], SHMEM_SIZE) != 0) {
    		perror("ftruncate shmem error");
    		err_flag++;
			goto END;
    	}

    	// подключаем общую память в адресное пространство 
    	if ((shmem_ptr[i] = mmap(0, SHMEM_SIZE, PROT_WRITE|PROT_READ,\
    						 MAP_SHARED, shmem_d[i], 0)) == MAP_FAILED) {
    		perror("mmap shmem error");
    		err_flag++;
			goto END;
    	}    	

    	// блокируем память
    	if (mlock(shmem_ptr[i], SHMEM_SIZE) != 0) {
    		perror("mlock shmem error");
    		err_flag++;
			goto END;    		
    	}

    	// зануляем ее
    	memset (shmem_ptr[i], 0, SHMEM_SIZE);
  	}	
	
	// создание потоков
	for (int i = 0; i < max_clients; i++) {
		pthread_args[i] = i;
		if (pthread_create(&threads_clients[i], NULL, func_thread_client,\
						  (void*) &pthread_args[i]) != 0) {
			printf(" Create threads failed");
			err_flag++;
			goto END;
		}		
	}

	// главный цикл работы
	while(main_while_var) {

		printf("Server: clients connected: %d\n", clients_connected_count);
		for (int i = 0; i < max_clients; i++) {
			printf("clients_info[%d].client_pid:%d\n", i, clients_info[i].client_pid);
		}
		sync();
		
		sleep(5);
	}

	// завершение программы сервера
	END:
	
	// ожидание завершения потоков
	stop_threads = 1;
	for (int i = 0; i < max_clients; i++) {	
		if (pthread_join(threads_clients[i], NULL) != 0) {
			printf(" pthread_join failed ");
			err_flag++;			
		}
	}

	for (int i = 0; i < max_clients * 2; i++) {
		
		// отделяем память от адресного пространства
		munmap(shmem_ptr[i], SHMEM_SIZE);

		// закрываем дескрипторы
		close(shmem_d[i]);

		// удаляем общую память
		shm_unlink(shmem_names[i]);
	}

	// закрываем и удаляем семафоры
	for (int i = 0; i < max_clients * 2; i++) {
		sem_close(sems[i]);
		sem_unlink(sem_names[i]);
	} 

	// освобождение памяти
	free(clients_info);	
	free(shmem_d);
	free(shmem_ptr);	
	free(sems);
	for (int i = 0; i < max_clients; i++) {	
		free(msg_to_cli[i]);
		free(msg_from_cli[i]);
	}
	free(msg_to_cli);
	free(msg_from_cli);
	free(threads_clients);
	free(pthread_args);
	for (int i = 0; i < max_clients * 2; i++) {
		free(shmem_names[i]);
		free(sem_names[i]);
	}
	free(shmem_names);
	free(sem_names);

	// выход 
	if (err_flag == 0) {
		exit (EXIT_SUCCESS);
	}
	else {
		exit(EXIT_FAILURE);
	}
}

// функция для потока по работе с клиентом
void* func_thread_client (void* arg){

	// принятый аргумент
	int* arg_num = (int*) arg;

	// считаем номер для участка памяти на прием
	int num_in = (*arg_num) * 2;

	// считаем номер для участка памяти на отправку
	int num_out = (*arg_num) * 2 + 1;

	// главный цикл потока
	while (stop_threads != 1) {
		

		// обработка входящих		 
		// блокировка семафора для своего входящего участка памяти
		// критическая секция
		sem_wait(sems[num_in]); 
		// вытаскиваем структуру из участка общей памяти
		memcpy(msg_from_cli[*arg_num], shmem_ptr[num_in],\
			   sizeof(struct_msg_client));
		memset(shmem_ptr[num_in], 0, sizeof(struct_msg_client));
		// конец критической секции
		sem_post(sems[num_in]);		

		// проверяем поле msg_buff, если там "connect"
		// то подключаем (pid берем из поля структуры flag)
		if (strncmp(msg_from_cli[*arg_num]->msg_buff, MSG_CONNECT, CON_COUNT) == 0) {
			if(clients_info[*(arg_num)].flag_connect == 0) {
				clients_info[*(arg_num)].client_pid = msg_from_cli[*arg_num]->flag;
				clients_info[*(arg_num)].flag_connect = 1;
				clients_connected_count++;
			}
		}

		// если там "disconnect", то отключаем
		else if ((strncmp(msg_from_cli[*arg_num]->msg_buff, MSG_DISCONNECT,\
							 DISCON_COUNT)) == 0 && (clients_info[*(arg_num)].flag_connect == 1)) {
			clients_info[*(arg_num)].client_pid = 0;
			clients_info[*(arg_num)].flag_connect = 0;
			clients_connected_count--;								
		}

		// в других случаях обрабатываем как входящее сообщение
		else {
				
			// если поле flag = 1, то сообщение новое- подлежит обработке
			if ((msg_from_cli[*arg_num]->flag == 1) && (clients_info[*(arg_num)].flag_connect == 1)) {
					
				printf("%s\n", msg_from_cli[*arg_num]->msg_buff);

				// кладем вх. структурку в исх.
				memcpy(msg_to_cli[*arg_num], msg_from_cli[*arg_num],\
						   sizeof(struct_msg_client));														
			}
		}				
		
		// обработка исходящих
		// если поле flag = 1, то сообщение подлежит обработке
		if ((msg_from_cli[*arg_num]->flag == 1) && (clients_info[*(arg_num)].flag_connect == 1)) {

			msg_from_cli[*arg_num]->flag = 0;
			for (int i = 1; i < max_clients * 2; i = i + 2) {
				// критическая секция		
				sem_trywait(sems[i]);			
				if (i != num_out) {
					// кладем структурку в общую память
					memcpy(shmem_ptr[i], msg_to_cli[*arg_num],\
				  		   sizeof(struct_msg_client));						
					}
				sem_post(sems[i]);				
			}
		}	
		memset(msg_from_cli[*arg_num], 0, sizeof(struct_msg_client));
	}
	pthread_exit(NULL);
}

void my_handler_CTRL_C (int sig) {
	main_while_var = 0;
}