#include "phone_book_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void create_new_empty_book (Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage){
	
	FILE* storage = fopen(phone_book_storage, "wb");

	if (storage) {
		fwrite(ptr_to_book, sizeof(Str_On_Book)*size_book, 1, storage);
	} 	
		else {
			printf("Create new book failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void load_book_from_storage (Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage){
	
	FILE* storage = fopen(phone_book_storage, "rb");

	if (storage) {
		fread(ptr_to_book, sizeof(Str_On_Book)*size_book, 1, storage);
	} 	
		else {
			printf("Load book from storage failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void add_new_string_to_book(Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage){


	// Поиск свободной структуры в массиве по флажку
	unsigned short index_for_add;
	
	for (int i = 0; i < size_book; i++){
		
		if (ptr_to_book[i].flag == 0){
			index_for_add = i;
			break;
		}
	}

	// Добавление записи в книгу
	printf("Founded a free page in a book # %d , please, enter the lastname\n", index_for_add);
	scanf("%s", ptr_to_book[index_for_add].last_name);
	printf("Enter the first name\n");
	scanf("%s", ptr_to_book[index_for_add].first_name);
	printf("Enter the phone number\n");
	scanf("%s", ptr_to_book[index_for_add].phone_number);

	ptr_to_book[index_for_add].flag = 1;

	printf("Thank you, string added to book\n");
	
	// Запись изменённой структуры в файл

	FILE* storage = fopen(phone_book_storage, "wb");

	if (storage) {
		fwrite(ptr_to_book, sizeof(Str_On_Book)*size_book, 1, storage);
	} 	
		else {
			printf("Open storage is failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void output_book_onDisplay(Str_On_Book* ptr_to_book, int size_book){

	unsigned short count_string_out = 0;

	for (int i = 0; i < size_book; i++){
		
		if (ptr_to_book[i].flag == 1){
			
			printf("   Last name: %s\n  First name: %s\nPhone number: %s\n\n", ptr_to_book[i].last_name, ptr_to_book[i].first_name, ptr_to_book[i].phone_number);
			count_string_out++;
		}
	}

	// Если в книге нет ни одной записи
	if (count_string_out == 0){
		printf("Phonebook is empty :(\n");
	}

}

void remove_string_from_book(Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage){

	unsigned short count_string_remove = 0;
	char lastname [30];

	printf("Enter the lastname for remove\n");
	scanf("%s", lastname);

	for (int i = 0; i < size_book; i++){
		if(strcmp(lastname, ptr_to_book[i].last_name) == 0){
			ptr_to_book[i].flag = 0;
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
		fwrite(ptr_to_book, sizeof(Str_On_Book)*size_book, 1, storage);
	} 	
		else {
			printf("Open storage is failed");
			exit(EXIT_FAILURE);
		}

	fclose(storage);
}

void search_string_with_lastname (Str_On_Book* ptr_to_book, int size_book){

	unsigned short count_string_search = 0;
	char lastname [30];

	printf("Enter the lastname for search\n");
	scanf("%s", lastname);

	for (int i = 0; i < size_book; i++){
		if(strcmp(lastname, ptr_to_book[i].last_name) == 0){
			printf("   Last name: %s\n  First name: %s\nPhone number: %s\n\n", ptr_to_book[i].last_name, ptr_to_book[i].first_name, ptr_to_book[i].phone_number);;
			count_string_search++;
		}
	}

	if (count_string_search == 0){
		printf("Strings not found\n");
	}	else {
			printf("Number of strings found - %d\n", count_string_search);
		}

}