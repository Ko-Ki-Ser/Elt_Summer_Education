all: libadd libsub libmul libdiv

add.o:	add.c
	gcc add.c -fPIC -c

sub.o:	sub.c
	gcc sub.c -fPIC -c

mul.o:	mul.c
	gcc mul.c -fPIC -c

div.o:	div.c
	gcc div.c -fPIC -c

libadd:	add.o
	gcc -shared -o libadd.so add.o 

libsub: sub.o
	gcc -shared -o libsub.so sub.o

libmul: mul.o
	gcc -shared -o libmul.so mul.o

libdiv: div.o
	gcc -shared -o libdiv.so div.o

clean:
	-rm add.o sub.o mul.o div.o libadd.so libsub.so libmul.so libdiv.so