#include "text_edit_func.h"
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>


int len (int lineno) {
       
    int linelen = COLS - 1;
 
    while (linelen >= 0 && mvinch (lineno, linelen) == ' ') {
        
        linelen--;
    }
    
    return linelen + 1;
}
   
// редактирование 
void edit_text () {
         
   	
    int c;
 
    while (1) {
        
       	move (row, col);
        	
        refresh ();
        	
        c = getch ();
 
        // команды редактора
        switch (c) {
 
            // стрелки: перемещают курсор
            case KEY_LEFT:
            	
            	if (col > 0) { 
            		col--;
            	}

            	else {
            		flash ();
            	}
            	
            	break;
 
            case KEY_DOWN:
            	
            	if (row < LINES - 1){
            		row++;	
            	} 
               
               	else {

               		flash ();
               	}
               
               	break;
 
            case KEY_UP:
               	
               	if (row > 0) {
               		
               		row--;
               	}
               
               	else {
               		
               		flash ();
               	}
               
               	break;
 
            case KEY_RIGHT:
               
               	if (col < COLS - 1) {
               		
               		col++;
               	}
               
               	else {
               		
               		flash ();
               	}

               	break;
 
            // i: переход в режим ввода
            case KEY_IC:
            case 'i':
               
             	input_symb ();
               	
               	break;
 
            // x: удалить текущий символ 
            case KEY_DC:
            case 'x':
               	
               	delch ();
               	
               	break;
 
            // o: вставить строку и перейти в режим ввода 
            case KEY_IL:
            case 'o':
            	
            	move (++row, col = 0);
               	
               	insertln ();
              	
              	input_symb ();
              	
              	break;
 
            // d: удалить текущую строку 
            case KEY_DL:
            case 'd':
               	
               	deleteln ();
               	
               	break;
 
            // CTRL+L: перерисовать экран
            case KEY_CLEAR:
            case CTRL('L'):
               
               	wrefresh (curscr);
               	
               	break;
 
            // s: записать и закончить работу 
            case 's':
              
              	return;
 
            // q: закончить работу без записи файла
            case 'q':
               	
               	endwin ();
               	
               	exit (EXIT_FAILURE);
 
            default:
               	
               	flash ();
               	
               	break;
        }
    }
}
 
// режим ввода: принимает и вставляет символы\
   выход: CTRL+D или EIC */
void input_symb () {
         
    int c;
 
    standout ();
        
    mvaddstr (LINES - 1, COLS - 20, "Режим ввода");
        
    standend ();
        
    move (row, col);
        
    refresh ();
        
    while (1) {
            
    	c = getch ();
            
        if (c == CTRL('D') || c == KEY_EIC) {

        	break;
        }
             
        insch (c);
            
        move (row, ++col);
            
        refresh ();
    }
           
    move (LINES - 1, COLS - 20);
        
    clrtoeol ();
        
    move (row, col);
        
    refresh ();
}  

// функция обработчик сигнала SIGWINCH
void sig_winch(int signo) {

	struct winsize size;

	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);

	resizeterm(size.ws_row, size.ws_col);
}