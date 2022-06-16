#include <stdio.h>

#define N 5


int main(){

	int arr[N][N];
	
	// Относительно главной диагонали

	printf("Above main diagonal\n\n");

	for (int i = 0; i < N; i++){
		for (int j = 0; j < i; j++){
			arr[i][j]= 0;
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = i; j < N; j++){
			arr[i][j]= 1;
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			printf("%2d%s", arr[i][j], " ");
		}
		printf("\n");
	}
	printf("\n\n");

	//Относительно побочной диагонали

	printf("Above secondary diagonal\n\n");

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N - i; j++){
			arr[i][j]= 1;
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = N - i; j < N; j++){
			arr[i][j]= 0;
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			printf("%2d%s", arr[i][j], " ");
		}
		printf("\n");
	}

	return 0;
}