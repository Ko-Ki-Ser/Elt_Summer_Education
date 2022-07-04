#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE_BUFF 100
#define SIZE_R_W 50
#define VALUE_CHANGING_OFFSET -25

int main (void) {
	
	system("clear");

	// буферы для чтения и записи
	char r_buff [SIZE_BUFF] = {0};
	char w_buff [SIZE_BUFF] = {0};

	// заполнение буфера на запись от 0 по возрастанию\
	для наглядности
	for (int i = 0; i < SIZE_BUFF; i++) {
		w_buff [i] = i;
	}

	// открытие 1ого файла с флагом O_RDWR (чтение-запись)
	int fd_1 = open ("file_1.txt", O_RDWR);

	// проверка возможной ошибки при открытии 1ого файла
	if (fd_1 == -1) {
		perror(" Open file_1.txt failed ");
		exit(EXIT_FAILURE);
	}

	// вывод буфера для записи
	printf("w_buff = ");
	for (int i = 0; i < SIZE_BUFF; i++) {
		if (w_buff [i] != 0) {
			printf("%d ", w_buff [i]);
		}
	}
	printf("\n\n");

	// запись в 1ый файл и проверка возможной ошибки, а также\
	вывод кол-ва записанных байт
	int w_count = 0;

	if ((w_count = write (fd_1, w_buff, SIZE_R_W)) == -1) {
		perror(" Write to file_1.txt failed ");
		exit(EXIT_FAILURE);
	}

	printf("\n%s%d\n", "count of writing bytes = ", w_count);

	// вывод размеров типов для понимания off_t	
	printf("\n%s%ld\n", "sizeof(off_t) = ", sizeof(off_t));
	printf("\n%s%ld\n", "sizeof(long) = ", sizeof(long));

	// смещение от начала файла в байтах после операции записи
	long offset_in_file_1 = lseek (fd_1 , 0, SEEK_CUR);

	// проверка возращаемого значения и вывод на экран
	if (offset_in_file_1 == -1) {
		perror(" lseek-cur in file_1 failed ");
		exit(EXIT_FAILURE);
	}

	printf("\n%s%ld\n", "offset after writing = ", offset_in_file_1);

	// чтение из файла без изменения смещения и проверка\
	возможной ошибки, а также вывод кол-ва считанных байт и буфера
	int r_count = 0;

	if ((r_count = read(fd_1, r_buff, SIZE_R_W)) == -1) {
		perror(" Read from file_1.txt without lseek failed ");
		exit(EXIT_FAILURE);
	}

	printf("\n%s%d\n", "count of reading bytes = ", r_count);

	printf("\nr_buff = ");
	for (int i = 0; i < SIZE_BUFF; i++) {
		if (r_buff [i] != 0) {
			printf("%d ", r_buff [i]);
		}
	}
	printf("\n");

	// вывод смещения после чтения
	printf("\n%s%ld\n", "offset after reading without lseek = ", \
	offset_in_file_1 = lseek(fd_1, 0, SEEK_CUR));

	// изменение смещения и вывод его в терминал
	printf("\n%s%d%s%ld\n", "value after changing offset on ", \
	VALUE_CHANGING_OFFSET, " = ", offset_in_file_1 = \
	lseek(fd_1, VALUE_CHANGING_OFFSET, SEEK_CUR));

	// чтение после изменения смещения с проверкой возможной\
	ошибки с выводом кол-ва считанных байт и смещения после чтения\
	без проверки результата функции lseek, и буфера для чтения
	if ((r_count = read(fd_1, r_buff, SIZE_R_W)) == -1) {
		perror(" Read from file_1.txt after lseek failed ");
		exit(EXIT_FAILURE);
	}

	printf("\n%s%d\n", "count of reading bytes = ", r_count);

	printf("\n%s%ld\n", "offset after reading after lseek = ",\
	offset_in_file_1 = lseek (fd_1 , 0, SEEK_CUR));

	printf("r_buff = ");
	for (int i = 0; i < SIZE_BUFF; i++) {
		if (r_buff [i] != 0) {
			printf("%d ", r_buff [i]);
		}
	}
	printf("\n");

	// снова операция чтения и вывод значений считанных\
	байт и смещения (read возвращает 0, когда достигнут конец\
	файла(EOF))
	if ((r_count = read(fd_1, r_buff, SIZE_R_W)) == -1) {
		perror(" Read from file_1.txt after lseek failed ");
		exit(EXIT_FAILURE);
	}

	printf("\n%s%d\n", "count of reading bytes (2) = ", r_count);
	printf("\n%s%ld\n", "offset after reading (2) = ",\
	offset_in_file_1 = lseek (fd_1 , 0, SEEK_CUR));

	close(fd_1);

	return 0;
}