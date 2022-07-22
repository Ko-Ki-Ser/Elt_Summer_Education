#ifndef TEXT_EDIT_FUNC_H
#define TEXT_EDIT_FUNC_H


// глобальные переменные положение текущего курсора
int row, col;

// функция обработчик сигнала SIGWINCH
void sig_winch(int signo);

// режим ввода: принимает и вставляет символы\
   выход: CTRL+D или EIC */
void input_symb ();

// редактирование 
void edit_text ();

int len (int lineno);

#endif