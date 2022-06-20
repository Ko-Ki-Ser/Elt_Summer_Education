all: phone_book.out

clean:
	-rm phone_book.out phone_book_main.o phone_book_func.o

phone_book_main.o: phone_book_main.c
	gcc -c phone_book_main.c

phone_book_func.o: phone_book_func.c
	gcc -c phone_book_func.c

phone_book.out: phone_book_main.o phone_book_func.o
	gcc -o phone_book.out phone_book_main.o phone_book_func.o

