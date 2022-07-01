#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define SIZE_LIB_NAME 100
#define SIZE_FUNC_NAME 100


int main (void) {

	system("clear");

	printf("\n Hello! Welcome to the calculator app based on the plugin\
 system.\n");

	// массивы и переменные для работы
	char lib_name_ar [SIZE_LIB_NAME] = {0};
	char func_name_ar [SIZE_FUNC_NAME] = {0};
	int (*func_ptr_main)(int, int) = NULL;
	int final_choice = 0;
	int arg1 = 0;
	int arg2 = 0;

	// метка для работы с другой библиотекой по желанию пользователя
	Start_again:

	printf("\n Please, enter the full name of the library in current\
 directory you want to load, for example ./libadd.so");

	// получение имени загружаемой библиотеки
	printf("\n\n Libname: ");

	if (scanf("%s", lib_name_ar) == EOF) {
		printf("\nReading libname failed");
		exit(EXIT_FAILURE);
	}

	// получение имени функции из загружаемой библиотеки для использования
	printf("\n Please, enter the name of function you want to use in\
 calculator, for example my_add\n\n Func_name: ");
	
	if (scanf("%s", func_name_ar) == EOF) {
		printf("\nReading func_name failed");
		exit(EXIT_FAILURE);
	}

	// загрузка библиотеки и получение указателя на функцию
	void* dl_obj = dlopen (lib_name_ar, RTLD_LAZY);

	if (dl_obj == NULL) {
		printf("\nOpen lib failed\n");
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	func_ptr_main = dlsym (dl_obj, func_name_ar);

	// если функция с таким именем не найдена
	if (func_ptr_main == NULL) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	// сама работа калькулятора с выбранными библиотекой и функцией
	printf("\n Enter arguments for calculate:\n\n");
	scanf("%d%d", &arg1, &arg2);
	printf("\n Result: %d", func_ptr_main(arg1, arg2));

	// поработать с новой библиотекой или завершить работу
	printf("\n Do you want to work with other library? Yes [1] No [2]");
	scanf("%d", &final_choice);

	if (final_choice == 1) {
		
		// подготовка массивов и переменных к загрузке новой библиотеки
		for (int i = 0; i < SIZE_LIB_NAME; i++) {
			lib_name_ar [i] = 0;
		}

		for (int i = 0; i < SIZE_FUNC_NAME; i++) {
			func_name_ar [i] = 0;
		}

		arg1 = arg2 = 0;
		func_ptr_main = NULL;
		dlclose(dl_obj);
		final_choice = 0;
		system ("clear");

		goto Start_again;
	}

		else if (final_choice == 2) {
			
			dlclose(dl_obj);
			system("clear");
			printf(" Thank you for use this little calculator\
! Goodbye!\n");

		} 
			else {

				dlclose(dl_obj);
				printf("\n Wrong final choice. Goodbye!\n");
				exit(EXIT_FAILURE);
			}
	
	return 0;
}