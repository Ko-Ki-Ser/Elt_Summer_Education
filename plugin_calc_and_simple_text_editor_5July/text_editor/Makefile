all: t_e.out 

text_ed.o: text_ed.c
	gcc text_ed.c -c

text_edit_func.o: text_edit_func.c
	gcc text_edit_func.c -c

t_e.out: text_ed.o text_edit_func.o
	gcc -o t_e.out text_ed.o text_edit_func.o -lncurses -g

clean:
	-rm t_e.out text_ed.o text_edit_func.o