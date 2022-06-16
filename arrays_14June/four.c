#include <stdio.h>

#define N 5

int main(){

	int arr[N][N];  // массив
	int number = 1; // число для записи в ячейку массива
	int i = 0;      // счетчик по строкам (сверху вниз) 
	int j = 0;      // счетчик по столбцам (слева направо) 

	int i_start = 0, i_end = 0, j_start = 0, j_end = 0; // отступы от углов самого большого внешнего квадрата

	printf("\n");

	while (number <= N*N){

		arr[i][j] = number;
        if (i == i_start && j < N - j_end - 1){  // Если заполняется верхняя сторона квадрата, то идем вправо
            ++j;
        }
        else if (j == N - j_end - 1 && i < N - i_end - 1){  // Если правая сторона, то идем вниз
            ++i;
        }
        else if (i == N - i_end - 1 && j > j_start){   // Если нижняя сторона, то идем влево 
            --j;
        }
        else {                                         // Левая сторона, идем вверх
            --i;
        }

        if ((i == i_start + 1) && (j == j_start) && (j_start != N - j_end - 1)){ // Если завершен периметр текущего квадрата,                                                         
            ++i_start;															 // то увеличиваем отступы и переходим к следующему
            ++i_end;
            ++j_start;
            ++j_end;
        }

        ++number;
	}

	for(int i = 0; i < N; i++){                     // Вывод
		for(int j = 0; j < N; j++){
			printf("%2d%s", arr[i][j], " ");
		}
		printf("\n");
	}

	printf("\n");

	return 0;
}