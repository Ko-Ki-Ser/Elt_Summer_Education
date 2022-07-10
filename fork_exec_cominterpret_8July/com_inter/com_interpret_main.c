#include <stdio.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <unistd.h>
#include "com_int_func.h"


int main(void) {

	int final_choice;

	system ("clear");
	printf("Hello! Welcome to a small simple command interpreter!\n\
Please enter the name of programm with args.\n");

	while (1) {

		comm_main();

		printf("\nDo you want to start any programm in this command\
interpreter? Yes[1] No[2]\n");
		scanf("%d", &final_choice);

		if (final_choice != 1) {
			break;
		}

		system("clear");
		final_choice = 0;

		// полный сброс stdin
		fflush(stdin);
		sync();
		__fpurge(stdin);

		printf("Please enter the name of programm with args.\n");
	}

	exit(EXIT_SUCCESS);
}