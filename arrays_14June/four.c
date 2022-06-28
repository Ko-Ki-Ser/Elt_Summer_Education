#include <stdio.h>

#define N 5

int main(){

	int arr[N][N];  // массив
	int number = 1; // число для записи в ячейку массива
	int i = 0;      // счетчик по строкам (сверху вниз) 
	int j = 0;      // счетчик по столбцам (слева направо) 

	// отступы от углов самого большого внешнего квадрата
	int i_start = 0, i_end = 0, j_start = 0, j_end = 0; 

	printf("\n");

	while (number <= N*N){

		arr[i][j] = number;

		// Если заполняется верхняя сторона квадрата, то идем вправо
        if (i == i_start && j < N - j_end - 1){  
            ++j;
        }
        // Если правая сторона, то идем вниз
        else if (j == N - j_end - 1 && i < N - i_end - 1){  
            ++i;
        }
        // Если нижняя сторона, то идем влево
        else if (i == N - i_end - 1 && j > j_start){    
            --j;
        }
        // Левая сторона, идем вверх
        else {                                         
            --i;
        }
		// Если завершен периметр текущего квадрата, 
        // то увеличиваем отступы и переходим к следующему
        ++i_end;
        ++j_start;

        if ((i == i_start + 1) && (j == j_start) && (j_start != N - j_end - 1)){                                                          
            ++i_start;															 
            ++j_end;
        }

        ++number;
	}

	// Вывод
	for(int i = 0; i < N; i++){                     
		for(int j = 0; j < N; j++){
			printf("%2d%s", arr[i][j], " ");
		}
		printf("\n");
	}

	printf("\n");

	return 0;
}