// Рабочая версия 2ого задания

#include <stdio.h>

#define M 10



int main(){

	int array[M];
	int content = 0;

	for (int i = 0; i < M; i++){
		array[i] = ++content;
	}

	for (int i = 0; i < M; i++){
		printf("%2d%s", array[i], " ");
	}

	printf("\n\n");

	for (int j = M - 1; j >= 0; j--){
		printf("%2d%s", array[j], " ");
	}

	printf("\n");
	
	return 0;
}