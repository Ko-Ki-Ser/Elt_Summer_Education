#include <stdio.h>
#include <stdlib.h>
#include "phone_book_func.h"


int main (int argc, char ** argv){


	/* Определение необходимого для работы размера телефонной книги и указателя
	на  этот размер */
 	int size_of_book = (FileSize(argv[1]) / (STR_SIZE*3)) + MEM_SIZE; 
	int* ptr_to_sizeofbook = &size_of_book;
	
	// Выделение памяти для загрузки массива структур из файла
	Str_On_Book* phone_book = calloc(size_of_book, sizeof(Str_On_Book));
	
	system("clear");
	
	TryAgainTheFirstChoice:
	
	printf("Create new empty book [1] or load book from storage [2] ?\n\
WARNING! If your choice is create new book it will destroy the existing book in \
file!\n");
	
	unsigned char the_first_choice;

	scanf("%hhu", &the_first_choice);
	
	if (the_first_choice == 1){
		create_new_empty_book(phone_book, argv[1]);
	} 
		else if (the_first_choice == 2){
			load_book_from_storage(phone_book, size_of_book, argv[1]);
		}
			else {
				system("clear");
				printf("Wrong choice, try again\n");
				goto TryAgainTheFirstChoice;
			}

	while(1){
		
		// Подсчет кол-ва занятых записей
		unsigned int count_of_using_strings = 0;
		
		for (int i = 0; i < size_of_book; i++){
			if (phone_book[i].flag == 1){
				count_of_using_strings++;
			}
		}

		// Если место заканчивается перевыделяем память
		if ((count_of_using_strings + 1) == size_of_book){
			Str_On_Book* phone_book = memory_reallocation(phone_book, ptr_to_sizeofbook, argv[1]);
		}
		
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
				add_new_string_to_book(phone_book, size_of_book, argv[1]);
				goto TryAgainTheFinalChoice;

			case 2:
				system("clear");
				remove_string_from_book(phone_book, size_of_book, argv[1]);
				goto TryAgainTheFinalChoice;

			case 3:
				system("clear");
				output_book_onDisplay(phone_book, size_of_book);
				goto TryAgainTheFinalChoice;
				
			case 4:
				system("clear");
				search_string_with_lastname(phone_book, size_of_book);
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

	free(phone_book);
	return 0;
}