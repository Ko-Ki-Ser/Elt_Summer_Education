#include <stdio.h>

#define N 5


int main(){

	int arr[N][N];
	int content = 0;

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			printf("%2d%s", arr[i][j]= ++content, " ");
		}
		printf("\n");
	}

	return 0;
}