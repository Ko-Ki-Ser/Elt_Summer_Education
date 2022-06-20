#ifndef PHONE_BOOK_FUNC_H
#define PHONE_BOOK_FUNC_H


// Структура записи в книге
typedef struct string_on_book {
	// Фамилия
	char last_name [30];
	// Имя
	char first_name [30];
	// Телефон
	char phone_number [30];
	// Флажок запись - свободна/занята
	char flag;
} Str_On_Book;


// Создание новой книги на 5 структур и запись её в файл для дальнейшей работы
void create_new_empty_book (Str_On_Book* ptr_to_book, const char* phone_book_storage); 

// Загрузка существующей книги из файла
void load_book_from_storage (Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage); 

// Добавление записи в книгу
void add_new_string_to_book (Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage); 

// Удаление записи из книги по фамилии (зануление флажка записи)
void remove_string_from_book (Str_On_Book* ptr_to_book, int size_book, const char* phone_book_storage); 

// Вывод книги на экран
void output_book_onDisplay (Str_On_Book* ptr_to_book, int size_book); 

// Поиск записи в книге по фамилии
void search_string_with_lastname (Str_On_Book* ptr_to_book, int size_book);

// Перевыделение памяти
Str_On_Book* memory_reallocation (Str_On_Book* ptr_to_old_book, int* ptr_to_sizeofbook,const char* phone_book_storage);

// Для определения размера файла
int FileSize(const char* file_name);

#endif