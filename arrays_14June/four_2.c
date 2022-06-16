// Реализация функции создания магического квадрата произвольного нечетного размера "Сиамским методом"
// Проверил его для n = 3 и n = 5

#include <stdio.h>
#include <stdlib.h>


int *create_magic_square(int n); 

int main (int argc, char **argv){

	if (argc != 2){
		printf("Invalid number of arguments\n");
		return -1;
	}

	int n = atoi(argv[1]);

	if (n == 2){
		printf("Magic square 2x2 is an exception\n");
		return -1;
	}
	
	int *ptr_magic_square = create_magic_square(n);

	if (ptr_magic_square == NULL){
		return -1;
	}

	for (int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			printf("%2d%s", *(ptr_magic_square + i*n + j), " ");
		}
		printf("\n");
	}

	free(ptr_magic_square);
	return 0;
}


int *create_magic_square(int n){

	int *starting_array = (int*)calloc(n*n, sizeof(int));
	
	int count = 1, y = 0, x = n/2;
    
    while (1){
        
        *(starting_array + y*n + x) = count;
        count++;
        
        if (((y == 0) && (x >= n-1)) && ((*(starting_array + (n-1)*n + 0)) != 0)){
            y++;
        }
        
        else {
            y--;
            
            if (y < 0) {
                y = n - 1;
            }
            
            x++;
                
            if (x == n) {
                x = 0;
            }
                
            if ((*(starting_array + y*n + x))!=0){
                y+=2;
                x--;
            }
        }

            if (count == n*n+1){ 
            	break;
            }
    }
    
	return starting_array;
}