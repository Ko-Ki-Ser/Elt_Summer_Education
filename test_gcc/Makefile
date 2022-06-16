all: test.i test.s test.o test


test.i:	test.c
	gcc -E test.c -o test.i

test.s: test.i
	gcc -S test.i

test.o: test.s
	gcc -c test.s

test:	test.o
	gcc -o test test.o