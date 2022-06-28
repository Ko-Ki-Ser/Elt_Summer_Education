all: calc.out

add.o:	add.c
	gcc add.c -c

sub.o:	sub.c
	gcc sub.c -c

mul.o:	mul.c
	gcc mul.c -c

div.o:	div.c
	gcc div.c -c

lib:	add.o sub.o mul.o div.o
	ar rc liboperations.a add.o sub.o mul.o div.o

calc.out: calc_main.c lib
	gcc calc_main.c -o calc.out -L. -loperations

clean:
	-rm calc.out add.o sub.o mul.o div.o liboperations.a