all: my_com_int.out

com_interpret_main.o: com_interpret_main.c
	gcc com_interpret_main.c -c

com_int_func.o: com_int_func.c
	gcc com_int_func.c -c

my_com_int.out: com_interpret_main.o com_int_func.o
	gcc -o my_com_int.out com_interpret_main.o com_int_func.o

clean:
	-rm my_com_int.out com_interpret_main.o com_int_func.o