#include <stdio.h>
#include <ncurses.h>


int main () {

	initscr();
	printw("HELLO");


	getch();
	endwin();
	return 0;
}