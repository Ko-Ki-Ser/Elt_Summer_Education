#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <mqueue.h>
#include <string.h>
#include <errno.h>

#define QUE_NAME "/myqueue"

#define PRIORITY 1

#define Q_SIZE 256
// при создании очереди сообщений с mq_attr.mq_maxmsg > 10
// вылетает ошибка EINVAL (Invalid argument)
#define MAX_MSG 10

int main (void) {

	// тип возвращаемый при создании
	mqd_t que;
	// сообщение для отправки клиенту
	char test_msg_out[] = "Hello! I'm server!";
	// структура для параметров очереди
	struct mq_attr que_attr;

	// максимальные число сообщений и размер очереди
	que_attr.mq_maxmsg = MAX_MSG;
	que_attr.mq_msgsize = Q_SIZE;

	// создание очереди с проверкой на ошибку
	if ((que = mq_open(QUE_NAME, O_CREAT|O_RDWR , 0777, &que_attr)) == (mqd_t)-1) {

    	perror(" creating queue failed ");
		exit(EXIT_FAILURE);
	}

	// отправка сообщения в очередь с проверкой на ошибку
	if (mq_send(que, test_msg_out, strlen(test_msg_out), PRIORITY) == -1) {
		perror(" sending msg failed ");
    	exit(EXIT_FAILURE);
	}

	// закрываем очередь с проверкой
	if (mq_close(que) == -1) {
		perror(" сlosing queue failed ");	
	}

	printf("Hello! I'm server! I send test msg to queue! Start client in\
other bash window\n");

	exit(EXIT_SUCCESS);
}