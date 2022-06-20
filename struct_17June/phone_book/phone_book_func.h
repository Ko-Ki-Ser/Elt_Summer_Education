#ifndef PHONE_BOOK_FUNC_H
#define PHONE_BOOK_FUNC_H

#define SIZE_BOOK 1000

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

#endif