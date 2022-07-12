#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>


// кол-во магазинов
#define COUNT_OF_SHOPS 5

// нижняя граница заполнения
#define SHOP_LOW 490

// верхня граница заполнения
#define SHOP_UP 510

// размер пополнения поставщиком
#define PROD_FILLING 500

// размер потребности покупателя
#define BUY_NEED 10000

// кол-во потоков-покупателей
#define COUNT_OF_BUY_THREADS 3

// кол-во потоков-поставщиков
#define COUNT_OF_PROD_THREADS 1

// время сна
#define TIME_OF_SLEEP 1


// функция для потоков-покупателей
void* func_buy(void* arg);

// функция для потоков-поставщиков
void* func_prod(void* arg);


//переменная для принудительного завершения потоков\
поставщиков
int stop_prod = 0;

// магазины
int shop[COUNT_OF_SHOPS];

// массив мьютексов для блокировок
pthread_mutex_t ml [COUNT_OF_SHOPS];


int main (void) {

	// инициализация магазинов
	for (int i = 0; i < COUNT_OF_SHOPS; i++) {

		shop[i] = SHOP_LOW + rand() % (SHOP_UP - SHOP_LOW + 1);
	}

	// инициализация мьютексов
	for (int i = 0; i < COUNT_OF_SHOPS; i++) {

		pthread_mutex_init(&ml[i], NULL);
	}

	// вывод номера магазина и его наполнения при старте
	printf("Main start:\n");

	for (int i = 0; i < COUNT_OF_SHOPS; i++) {

		printf("Shop [%d] = %d\n", i + 1, shop[i]);

		if (i == (COUNT_OF_SHOPS - 1)) {
			printf("\n");
		}
	}
	
	// массив для хранения id потоков-покупателей
	pthread_t thread_buy [COUNT_OF_BUY_THREADS];

	// создание потоков-покупателей
	for (int i = 0; i < COUNT_OF_BUY_THREADS; i++) {
		
		if (pthread_create(&thread_buy[i], NULL, func_buy, NULL) != 0) {

			printf(" Create thread_buy %d failed", i);
			fflush(stdout);
			sync();
			exit(EXIT_FAILURE);
		}
	}

	// массив для хранения id потоков-поставщиков
	pthread_t thread_prod [COUNT_OF_PROD_THREADS];

	// создание потоков-поставщиков
	for (int i = 0; i < COUNT_OF_PROD_THREADS; i++) {

		if (pthread_create(&thread_prod[i], NULL, func_prod, NULL) != 0) {

			printf(" Create thread_prod %d failed", i);
			fflush(stdout);
			sync();
			exit(EXIT_FAILURE);
		}
	}

	// ждем завершения потоков-покупателей
	for (int i = 0; i < COUNT_OF_BUY_THREADS; i++) {
		
		if (pthread_join(thread_buy[i], NULL) != 0) {

			printf(" Join thread_buy %d failed", i);
			fflush(stdout);
			sync();
			exit(EXIT_FAILURE);
		}
	}

	// принудительно завершаем потоки-поставщики,\
	выставляя переменную stop_prod в 1
	stop_prod = 1;

	// вывод номера магазина и его наполнения при завершении
	printf("\nMain finish:\n");

	for (int i = 0; i < COUNT_OF_SHOPS; i++) {

		printf("Shop [%d] = %d\n", i + 1, shop[i]);

		if (i == (COUNT_OF_SHOPS - 1)) {
			printf("\n");
		}
	}

	// уничтожение мьютексов при завершении
	for (int i = 0; i < COUNT_OF_SHOPS; i++) {

		pthread_mutex_destroy(&ml[i]);		
	}
	
	exit(EXIT_SUCCESS);
}


void* func_buy(void* arg) {

	// получения идентификатора потока
	pid_t tid;
	tid = syscall(SYS_gettid);

	// потребность покупателя
	int buy_need = BUY_NEED;

	while (1) {

		//пробегаем магазины
		for (int i = 0; i < COUNT_OF_SHOPS; i++) {

			//условие завершения потока
			if (buy_need == 0) {
				break;
			}

			// пытаемся захватить блокировку и при удачной\
			попытке опустошаем магазин, удовлетворяя свою\
			потребность покупателя
			if (pthread_mutex_trylock(&ml[i]) == 0) {

				printf("Buyer [%d] woke up and lock shop[%d]=%d, \
buy_need is %d\n", tid, i + 1, shop[i], buy_need);
				buy_need = buy_need - shop[i];
				shop[i] = 0;

				if (buy_need < 0) {

					buy_need = 0;
				}

				pthread_mutex_unlock(&ml[i]);
				printf("Buyer [%d] fell asleep and unlock shop[%d]=%d, \
buy_need is %d\n", tid, i + 1, shop[i], buy_need);
				sleep(TIME_OF_SLEEP);
			}
		}

		// условие завершения потока
		if (buy_need == 0) {
			break;
		}
	}

	printf("Buy_need of buyer [%d] in finish is %d\n", tid, buy_need);
	fflush(stdout);
	sync();
	pthread_exit(NULL);
}

void* func_prod(void* arg) {

	// получения идентификатора потока
	pid_t tid;
	tid = syscall(SYS_gettid);

	while (1) {

		//пробегаем магазины
		for (int i = 0; i < COUNT_OF_SHOPS; i++) {

			// условие завершения потока
			if (stop_prod) {
				break;
			}

			// пытаемся захватить блокировку и при удачной\
			попытке пополняем магазин
			if (pthread_mutex_trylock(&ml[i]) == 0) {

				printf("Producer [%d] woke up and lock shop[%d]=%d\n", tid,\
						i + 1, shop[i]);
				shop[i] = shop[i] + PROD_FILLING;
				pthread_mutex_unlock(&ml[i]);
				printf("Producer [%d] fell asleep and unlock shop[%d]=%d\n", tid,\
						i + 1, shop[i]);
				sleep(TIME_OF_SLEEP);
			}
		}

		// условие завершения потока
		if (stop_prod) {
			break;
		}
	}

	pthread_exit(NULL);	
}