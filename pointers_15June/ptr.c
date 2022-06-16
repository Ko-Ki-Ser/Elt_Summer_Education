#include <stdio.h>

#define N 30000
#define M 5


int main(){

	int a = N;
	int* ptr_to_int = &a;
	char* ptr_to_char = (char*)&a;
	
	// вывод размера типов в байтах
	printf("%ld\t%ld\n\n", sizeof(char), sizeof(int)); 
	
	// вывод адреса на int, на char для сравнения
	printf("%p\n\n%p\n\n", ptr_to_int, ptr_to_char); 
	
	// вывод значений каждого байта int a
	printf("%d\t%d\t%d\t%d\t%d\t\n", *ptr_to_int, *ptr_to_char,*(ptr_to_char + 1), *(ptr_to_char + 2), *(ptr_to_char + 3)); 
	
	// изменение значения 3ого байта int a
	char* ptr_one = ptr_to_char + 2; 
	*(ptr_one) = M;

	// вывод нового значения int a
	printf("%d\n", *(ptr_to_int)); 

	return 0;
}