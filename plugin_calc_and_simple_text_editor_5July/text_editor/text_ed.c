#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <signal.h>
#include <locale.h>

#include "text_edit_func.h"


int main (int argc, char** argv) {

	setlocale(LC_ALL, "");

    // необходимые переменные для работы редактора

	int i, n, l;
    int c;
    int line = 0;
    
	FILE* fd_file = NULL;

	// проверка кол-ва аргументов при старте
	if (argc != 2) {
		printf("Invalid number of arguments");
		exit(EXIT_FAILURE);
	}

	// открываем файл
	fd_file = fopen (argv [1], "r");

	// проверка возвр. значения
	if (fd_file == NULL) {
		printf("Opening file failed");
		exit(EXIT_FAILURE);
	}

	// инициализирует структуры данных ncurses и переводит\
	терминал в нужный режим
	initscr ();

	// обработчик SIGWINCH
	signal(SIGWINCH, sig_winch);

	//cbreak режим, веденные пользователем символы немедленно поступают\
	в приложение, функции curses не выполняют специальную\
	обработку символов стирания и уничтожения.
    cbreak ();

    //Определяет, должны ли функции curses преобразовывать символ начала\
    строки в символы возврата каретки и перевода строки (при выводе) и\
    символ Return в символ начала строки (при вводе)
    nonl ();

    // отключает отображение символов, вводимых с клавиатуры
    noecho ();

    // для вставки/удаления строки
    idlok (stdscr, TRUE);

    // для функциональных клавиш клавиатуры (стрелок)
    keypad (stdscr, TRUE);
 
    // читаем файл
    while ((c = getc(fd_file)) != EOF) {
    	if (c == '\n') {
    		line++;
    	}
    
    //	if (line > LINES - 2) {
    //		break;
    //	}
    
    	// для вывода одного символа, переданного в параметре
    	addch(c);
    }
    
    fclose (fd_file);

    move (0, 0);
    
    refresh ();
    
    edit_text ();
 
    // запись изменений в файл
    fd_file = fopen (argv [1], "w");
    
    for (l = 0; l < LINES - 1; l++) {
        
        n = len(l);
        
        for (i = 0; i < n; i++) {
            
            putc (mvinch (l, i) & A_CHARTEXT, fd_file);
        }   
        
        putc('\n', fd_file);
    }
         
    fclose(fd_file);
 
    endwin ();

    return 0;
}
 
// режим редактора
// стрелки: перемещают курсор
// s: записать и закончить работу
// q: закончить работу без записи файла
// CTRL+L: перерисовать экран
// d: удалить текущую строку
// o: вставить строку и перейти в режим ввода
// режим ввода: принимает и вставляет символы\
   выход: CTRL+D или EIC */
// x: удалить текущий символ
// i: переход в режим ввода