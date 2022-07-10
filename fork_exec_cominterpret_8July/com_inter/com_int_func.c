#include "com_int_func.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define PARS_COMM_STR_BUFSIZE 128
#define ESCAPE_SEQS " \t\r\n\a"

// главная функция командного интерпретатора
void comm_main (void) {

	char* command_str;
	char** args;
	
		// читаем строку
	   	command_str = read_comm_str();

	   	// парсим её
    	args = parsing_comm_str(command_str);
    	
    	// запускаем процесс
    	start_comm(args);

    	free(command_str);
    	free(args);  	
}

// функция для считывания строки
char* read_comm_str () {

	char* res = NULL;
  	
  	// getline сама перевыделит память, используя realloc()
  	ssize_t size = 0;
  	
  	if (getline(&res, &size, stdin) == -1) {

  		perror(" read_comm_str failed ");
  		exit(EXIT_FAILURE);
  	}
  	
  	return res;
}

// функция парсинга на имя и аргументы
char** parsing_comm_str (char* command_str) {

	int buf_size = PARS_COMM_STR_BUFSIZE; 
	int pos = 0;

  	char** words = malloc(buf_size * sizeof(char*));
  	char* word;

  	if (words == NULL) {

    	printf("parsing_comm_str: ошибка в malloc\n");
    	exit(EXIT_FAILURE);
  	}

  	word = strtok(command_str, ESCAPE_SEQS);

  	while (word != NULL) {

    	words[pos] = word;
    	pos++;

    	if (pos >= buf_size) {

      		buf_size = buf_size + PARS_COMM_STR_BUFSIZE;
      		words = realloc(words, buf_size * sizeof(char*));
      
      		if (words == NULL) {

        		printf("parsing_comm_str: ошибка в realloc\n");
        		exit(EXIT_FAILURE);
      		}
    	}

    	word = strtok(NULL, ESCAPE_SEQS);
  	}
  	
  	words[pos] = NULL;

  	return words;
}

// функция запуска процесса
void start_comm(char** args) {

	pid_t pid = fork();

  	// родительский процесс
  	if (pid != 0 && pid != -1) {
    
    	waitpid(pid, 0, 0);    	        	
  	} 
  		// порожденный процесс
		else if (pid == 0) {
    		
    		// execvp (v)-предоставляет процессу массив указателей на строки,\
    		   заканчивающиеся на null, первый элемент - имя исполняемого\
    		   модуля. (p)-ОС ищет исп. файл самостоятельно
    		if (execvp(args[0], args) == -1) {
      			
      			perror(" execvp in start_comm ");
      			exit(EXIT_FAILURE);
    		}
  		}
  			// ошибка порождения процесса
  			else if (pid == -1) {

  				perror(" fork in start_comm ");
      			exit(EXIT_FAILURE);
  			} 
}