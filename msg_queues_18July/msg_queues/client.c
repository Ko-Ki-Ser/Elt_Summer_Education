#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>       /* For O_* constants */
#include <sys/stat.h>    /* For mode constants */
#include <mqueue.h>
#include <string.h>

#define QUE_NAME "/myqueue"

#define PRIORITY 1

#define Q_SIZE 256


int main (void) {

	mqd_t que;
	char test_msg_in[Q_SIZE];
	int priority;

	// открытие очереди с проверкой
	if ((que = mq_open(QUE_NAME, O_RDWR , 0777, NULL)) == (mqd_t)-1) {
    	perror(" opening queue failed ");
		exit(EXIT_FAILURE);
	}

	// принимаем сообщение из очереди
	if (mq_receive(que, test_msg_in, Q_SIZE, &priority) == -1) {
		perror(" receiving msg failed ");
		exit(EXIT_FAILURE);
	}

	// закрываем очередь с проверкой
	if (mq_close(que) == -1) {
		perror(" сlosing queue failed ");	
	}

	// удаляем очередь из системы
	if (mq_unlink(QUE_NAME) == -1) {
		perror(" removing queue failed ");
		exit(EXIT_FAILURE);
	}

	printf("Hello! I'm client!\nReceived msg from server: %s\n", test_msg_in);

	exit(EXIT_SUCCESS);
}