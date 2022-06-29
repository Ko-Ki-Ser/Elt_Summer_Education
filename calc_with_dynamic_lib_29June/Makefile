all: calc.out

add.o:	add.c
	gcc add.c -fPIC -c

sub.o:	sub.c
	gcc sub.c -fPIC -c

mul.o:	mul.c
	gcc mul.c -fPIC -c

div.o:	div.c
	gcc div.c -fPIC -c

lib:	add.o sub.o mul.o div.o
	gcc -shared -o liboperations.so add.o sub.o mul.o div.o

calc.out: calc_main.c lib
	gcc calc_main.c -o calc.out -L. -loperations

clean:
	-rm calc.out add.o sub.o mul.o div.o liboperations.so