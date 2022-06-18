#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BOOK 1000

// Создание новой книги и запись её в файл для дальнейшей работы
void create_new_empty_book (const char* phone_book_storage); 

// Загрузка существующей книги из файла
void load_book_from_storage (const char* phone_book_storage); 

// Добавление записи в книгу
void add_new_string_to_book (const char* phone_book_storage); 

// Удаление записи из книги по фамилии (зануление флажка записи)
void remove_string_from_book (const char* phone_book_storage); 

// Вывод книги на экран
void output_book_onDisplay (); 

// Поиск записи в книге по фамилии
void search_string_with_lastname (); 

// Структура записи в книге
struct string_on_book {
	// Фамилия
	char last_name [30];
	// Имя
	char first_name [30];
	// Телефон
	char phone_number [30];
	// Флажок запись - свободна/занята
	char flag;
};

// Массив структур, т.е. сама книга
struct string_on_book phone_book [SIZE_BOOK];


int main (void){
	
	system("clear");
	
	TryAgainTheFirstChoice:
	
	printf("Create new empty book [1] or load book from storage [2] ?\n\
WARNING! If your choice is create new book it will destroy the existing book in file!\n");
	
	unsigned char the_first_choice;
	
	scanf("%hhu", &the_first_choice);
	
	if (the_first_choice == 1){
		create_new_empty_book("phone_book_storage");
	} 
		else if (the_first_choice == 2){
			load_book_from_storage("phone_book_storage");
		}
			else {
				system("clear");
				printf("Wrong choice, try again\n");
				goto TryAgainTheFirstChoice;
			}

	while(1){
		
		system("clear");

		TryAgainTheSecondChoice:

		printf("Menu:\n \
		[1] - Add new string to phone book\n \
		[2] - Remove string from phone_book\n \
		[3] - Show all strings on screen\n \
		[4] - Search in book with lastname\n");

		unsigned char the_second_choice;

		scanf("%hhu", &the_second_choice);
		
		switch (the_second_choice) {

			case 1:
				system("clear");
				add_new_string_to_book("phone_book_storage");
				break;

			case 2:
				system("clear");
				remove_string_from_book("phone_book_storage");
				goto TryAgainTheFinalChoice;

			case 3:
				system("clear");
				output_book_onDisplay();
				goto TryAgainTheFinalChoice;
				
			case 4:
				system("clear");
				search_string_with_lastname();
				goto TryAgainTheFinalChoice;

			default:
				system("clear");
				printf("Wrong choice, try again\n");
				goto TryAgainTheSecondChoice;
		}

		system("clear");

		TryAgainTheFinalChoice:
		printf("Do you want return to menu [1] or exit [2]?\n");

		unsigned char the_final_choice;

		scanf("%hhu", &the_final_choice);

		if (the_final_choice == 1){
			system("clear");
			continue;
		}
			else if (the_final_choice == 2){
				system ("clear");
				printf("Thank you for watching this little programm :) Goodbye!\n");
				break;
			}
				else {
					system("clear");
					printf("Wrong choice, try again\n");
					goto TryAgainTheFinalChoice;
				}
	
	}

	return 0;
}


// Определение объявленных выше функций

void create_new_empty_book (const char* phone_book_storage){
	
	FILE* storage = fopen(phone_book_storage, "wb");

	if (storage) {
		fwrite(&phone_book, sizeof(phone_book), 1, storage);
	} 	
		else {
			printf("Create new book failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void load_book_from_storage (const char* phone_book_storage){
	
	FILE* storage = fopen(phone_book_storage, "rb");

	if (storage) {
		fread(&phone_book, sizeof(phone_book), 1, storage);
	} 	
		else {
			printf("Load book from storage failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void add_new_string_to_book(const char* phone_book_storage){


	// Поиск свободной структуры в массиве по флажку
	unsigned short index_for_add;
	
	for (int i = 0; i < SIZE_BOOK; i++){
		
		if (phone_book[i].flag == 0){
			index_for_add = i;
			break;
		}
	}

	// Добавление записи в книгу
	printf("Founded a free page in a book # %d , please, enter the lastname\n", index_for_add);
	scanf("%s", phone_book[index_for_add].last_name);
	printf("Enter the first name\n");
	scanf("%s", phone_book[index_for_add].first_name);
	printf("Enter the phone number\n");
	scanf("%s", phone_book[index_for_add].phone_number);

	phone_book[index_for_add].flag = 1;

	printf("Thank you, string added to book\n");
	
	// Запись изменённой структуры в файл

	FILE* storage = fopen(phone_book_storage, "wb");

	if (storage) {
		fwrite(&phone_book, sizeof(phone_book), 1, storage);
	} 	
		else {
			printf("Open storage is failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void output_book_onDisplay(){

	unsigned short count_string_out = 0;

	for (int i = 0; i < SIZE_BOOK; i++){
		
		if (phone_book[i].flag == 1){
			
			printf("   Last name: %s\n  First name: %s\nPhone number: %s\n\n", phone_book[i].last_name, phone_book[i].first_name, phone_book[i].phone_number);
			count_string_out++;
		}
	}

	// Если в книге нет ни одной записи
	if (count_string_out == 0){
		printf("Phonebook is empty :(\n");
	}

}

void remove_string_from_book(const char* phone_book_storage){

	unsigned short count_string_remove = 0;
	char lastname [30];

	printf("Enter the lastname for remove\n");
	scanf("%s", lastname);

	for (int i = 0; i < SIZE_BOOK; i++){
		if(strcmp(lastname, phone_book[i].last_name) == 0){
			phone_book[i].flag = 0;
			count_string_remove++;
		}
	}

	if (count_string_remove == 0){
		printf("No strings to delete\n");
	}	else {
			printf("Removal was successful. Number of deleted strings - %d\n", count_string_remove);
		}

	// Запись изменённой структуры в файл

	FILE* storage = fopen(phone_book_storage, "wb");

	if (storage) {
		fwrite(&phone_book, sizeof(phone_book), 1, storage);
	} 	
		else {
			printf("Open storage is failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void search_string_with_lastname (){

	unsigned short count_string_search = 0;
	char lastname [30];

	printf("Enter the lastname for search\n");
	scanf("%s", lastname);

	for (int i = 0; i < SIZE_BOOK; i++){
		if(strcmp(lastname, phone_book[i].last_name) == 0){
			printf("   Last name: %s\n  First name: %s\nPhone number: %s\n\n", phone_book[i].last_name, phone_book[i].first_name, phone_book[i].phone_number);;
			count_string_search++;
		}
	}

	if (count_string_search == 0){
		printf("Strings not found\n");
	}	else {
			printf("Number of strings found - %d\n", count_string_search);
		}

}