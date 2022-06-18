#include <stdio.h>


int main (void){

	struct test_v1 {
		char a_v1;
		int b_v1;
	};

	struct test_v2 {
		char a_v2;
		int b_v2;
	} __attribute__((packed));

	// Вывод размеров структур для наглядности
	printf("sizeof struct test_v1 = %lu  sizeof struct test_v2 = %lu\n\n\n", sizeof(struct test_v1), sizeof(struct test_v2));

	// Статический массив для экспериментов со структурами
	char str[10] = {'A', 0, 0, 0, 0, 'B', 0, 0, 0, 0};

	// Объявление экземпляров структур (без инициализации полей (это важно))
	struct test_v1 test_v1;
	struct test_v2 test_v2;

	struct test_v1* ptr_v1;
	struct test_v2* ptr_v2; 

	ptr_v1 = (struct test_v1*) str;
	ptr_v2 = (struct test_v2*) str;

	printf("Вывод памяти по которой лежит массив через указатели на структуры\n\n");
	printf("ptr_v1->a_v1 = %c \t ptr_v1->b_v1 = %d \n\n", ptr_v1->a_v1, ptr_v1->b_v1); 
	printf("ptr_v2->a_v2 = %c \t ptr_v2->b_v2 = %d \n\n", ptr_v2->a_v2, ptr_v2->b_v2);

	printf("Вывод значений полей экземпляров структур через точку (здесь мусор(т.к. не было инициализации))\n\n");
	printf("test_v1.a_v1 = %c \t test_v1.b_v1 = %d \n\n", test_v1.a_v1, test_v1.b_v1); 
	printf("test_v2.a_v2 = %c \t test_v2.b_v2 = %d \n\n\n", test_v2.a_v2, test_v2.b_v2);

	char* ptr_to_char_v1 = (char*) ptr_v1;
	char* ptr_to_char_v2 = (char*) ptr_v2;

	printf("Побайтовый вывод через указатели\n\n");

	for (int i = 0; i < sizeof(struct test_v1); i++){

		printf("*(ptr_to_char_v1 +%d) = %c\n", i, *(ptr_to_char_v1 + i) );
	}

	printf("\n\n");

	for (int j = 0; j < sizeof(struct test_v2); j++){

		printf("*(ptr_to_char_v2 +%d) = %c\n", j, *(ptr_to_char_v2 + j) );
	}

	printf("\n\n");

	return 0;
}