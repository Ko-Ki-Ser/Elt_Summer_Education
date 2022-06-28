#include "operations.h"
#include <stdio.h>
#include <stdlib.h>


int main (void){

	system("clear");

	int choice_operation, arg1, arg2, the_final_choice;
	
	while (1){

		system("clear");

		choice_operation = arg1 = arg2 = 0;

		printf("Hello!\nSelect arithmetic operation, please\n\n\
1. ADD\n2. SUB\n3. MUL\n4. DIV\n");

		scanf("%d", &choice_operation);

		if (choice_operation != 1 && choice_operation != 2 && \
			choice_operation != 3 && choice_operation != 4) {
						
			continue;
		}

		printf("\nEnter two arguments for the selected operation, please\n\n");

		scanf("%d%d", &arg1, &arg2);

		switch (choice_operation) {

			case 1:
				printf("%d + %d = %d\n", arg1, arg2, my_add(arg1, arg2));
				break;

			case 2:
				printf("%d - %d = %d\n", arg1, arg2, my_sub(arg1, arg2));
				break;

			case 3:
				printf("%d * %d = %d\n", arg1, arg2, my_mul(arg1, arg2));
				break;

			case 4:
				printf("%d / %d = %d\n", arg1, arg2, my_div(arg1, arg2));
				break;

			default:
				printf("Something went wrong...\n");
				exit(EXIT_FAILURE);
		}	

		Cont_OR_exit:
		printf("\n\nDo you want to try again? Yes[1]  No[2]\n");

		scanf("%d", &the_final_choice);
		
		if (the_final_choice == 1){
			continue;
		}

			else if (the_final_choice == 2) {

				system("clear");
				printf("Thank you! Goodbye!\n");
				break;
			}

				else {
					
					printf("Wrong choice, try again, please\n");
					system("clear");
					goto Cont_OR_exit;
				}
	}

	return 0;
}