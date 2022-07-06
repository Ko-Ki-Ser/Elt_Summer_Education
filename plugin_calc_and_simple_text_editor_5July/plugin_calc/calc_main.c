#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include "struct_func_info.h"

#define PATH_TO_PLUGINS "/home/kirill/github repositories/Eltex_Education/\
//plugin_calc_and_simple_text_editor_5July/plugin_calc/plugins/"

#define STRUCT_NAME "f_info"
#define NAME_FILE_BUF "file_buffer"
#define COUNT_FILENAMES 50
#define LEN_FUNC_NAME 20

char path_to_plugins [1024] = {PATH_TO_PLUGINS};


int main (void) {

	// очищаем консоль
	system("clear");

	// открываем каталог с плагинами
	DIR* plugins_dir = opendir (path_to_plugins);

	// обработка возможной ошибки
	if (plugins_dir == NULL) {
		perror (" Opendir failed ");
		exit (EXIT_FAILURE);
	}

	// указатель для считывания содержимого каталога\
	массив имен файлов и счетчик кол-ва нужных имен
	struct dirent* cur_file = NULL;
	char* filename [COUNT_FILENAMES] = {0};
	int count_value_filenames = 0;

	// читаем каталог с плагинами считаем количество нужных библиотек\
	и сохраняем их имена в массив
	while ((cur_file = readdir(plugins_dir)) != NULL) {

		if (cur_file->d_type == DT_REG) {
			if (strncmp("lib", cur_file->d_name, 3) == 0) {
				filename [count_value_filenames] = cur_file->d_name;
				count_value_filenames++;
			}
		}
	}

	closedir(plugins_dir);

	// массив указателей для загрузки библиотек
	char** dl_obj = calloc (count_value_filenames, sizeof(void*));
	
	// массив указателей на структуры
	struct func_info** array_func_inf = calloc (count_value_filenames, sizeof(void*));
	
	for (int i = 0; i < count_value_filenames; i++) {
		array_func_inf [i] = malloc (sizeof(struct func_info));
	}

	// указатель для копирования структур из библиотек
	struct func_info* ptr_to_struct = NULL;
	
	// открываем все найденные библиотеки для считывания структур\
	описывающих функции в этих библиотеках

	for (int i = 0; i < count_value_filenames; i++) {
		
		// открываем библиотеки
		dl_obj [i] = (char*) dlopen (strcat(path_to_plugins, filename [i]),\
		RTLD_LAZY);

		strcpy(path_to_plugins, PATH_TO_PLUGINS);
		
		if (dl_obj [i] == NULL) {
			printf("\nOpen lib %s failed\n", filename[i]);
			fprintf(stderr, "%s\n", dlerror());
						
			free(dl_obj);
			
			for (int i = 0; i < count_value_filenames; i++) {
				free(array_func_inf[i]);
			}
			free(array_func_inf);
			
			exit(EXIT_FAILURE);
		}

		// получаем указатель на структуру 
		ptr_to_struct = dlsym(dl_obj [i], STRUCT_NAME);

		if (ptr_to_struct == NULL){

			fprintf(stderr, "%s\n", dlerror());
						
			free(dl_obj);
			
			for (int i = 0; i < count_value_filenames; i++) {
				free(array_func_inf[i]);
			}
			free(array_func_inf);
			
			exit(EXIT_FAILURE);
		}

	// копируем структуру			
	memcpy(array_func_inf[i], ptr_to_struct, sizeof(struct func_info));

	}

	// выделяем память под массив указателей на функции

	// (упрощение: без проверки числа аргументов, их типа и типа возвр. значения\
	будем считать что все функции возвращают int и принимают два аргумента типа\
	int)

	int (**arr_ptr_to_funcs)(int, int) = calloc (count_value_filenames,\
	sizeof(void*));

	// получаем адреса функций из присоединенных библиотек
	for (int i = 0; i < count_value_filenames; i++) {

		arr_ptr_to_funcs [i] = dlsym(dl_obj [i], array_func_inf [i]->func_name);

		if (arr_ptr_to_funcs [i] == NULL){
			
			fprintf(stderr, "%s\n", dlerror());
						
			free(dl_obj);
			
			for (int i = 0; i < count_value_filenames; i++) {
				free(array_func_inf[i]);
			}
			free(array_func_inf);

			free(arr_ptr_to_funcs);
			
			exit(EXIT_FAILURE);
		}

	}

	// начало взаимодействия с пользователем
	char choice_func [LEN_FUNC_NAME] = {0};
	int arg1;
	int arg2;
	int final_choice;

	printf(" Hello! If this message show on screen, this little calc loaded all\
 lib***.so from %s\n\n", PATH_TO_PLUGINS);
	
	WorkAgain:

	system("clear");

	printf("List of loaded libs and available functions:\n\n");

	for (int i = 0; i < count_value_filenames; i++)	{
		printf("%s\n", filename [i]);
		printf("%s [%d]\n\n", array_func_inf [i]->func_name, i + 1);
	}
	
	printf("Please, select the function to work and enter func name\n");
	scanf("%s", choice_func);
	printf ("Please, enter args\n");
	scanf("%d%d", &arg1, &arg2);

	for (int i = 0; i < count_value_filenames; i++) {
		if (strcmp(choice_func, array_func_inf [i]->func_name) == 0) {
			printf("Result: %d\n\n", arr_ptr_to_funcs[i](arg1, arg2));
		}
	}

	printf("Do you want choose other func? Yes[1] No[2]");
	scanf("%d", &final_choice);

	if (final_choice == 1){
		goto WorkAgain;
	} 
	
	// завершение программы и освобождение ресурсов
	
	free(dl_obj);
			
	for (int i = 0; i < count_value_filenames; i++) {
		free(array_func_inf[i]);
	}
	free(array_func_inf);
	
	free(arr_ptr_to_funcs);

	return 0;
}