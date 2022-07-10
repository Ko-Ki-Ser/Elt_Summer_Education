/* Дерево порождения процессов. 
(1)->(2),(3)
(2)->(4)
(3)->(5),(6)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main () {

system("clear");

	
	pid_t ret_2 = 0;
	pid_t ret_3 = 0;
	pid_t ret_4 = 0;
	pid_t ret_5 = 0;
	pid_t ret_6 = 0;

	printf ("It's procces(1)! PPID = %d, PID = %d\n\n",\
		 	getppid(), getpid());

	// порождение второго процесса (1)->(2)
	ret_2 = fork();

	// код первого процесса (1)
	if (ret_2 != 0 && ret_2 != -1) {
				
		// порождение третьего процесса (1)->(3)
		ret_3 = fork();

		// код первого процесса (1)
		if (ret_3 != 0 && ret_3 != -1) {
			
		}
			// находимся в третьем процессе (3)
			else if (ret_3 == 0) {
				printf ("It's procces(3)! PPID = %d, PID = %d\n\n",\
		 		getppid(), getpid());

				// порождаем 5ый процесс (3)->(5)
		 		ret_5 = fork();

		 		// код третьего процесса (3)
		 		if (ret_5 != 0 && ret_5 != -1) {
		 			
		 			// порождаем шестой процесс (3)->(6)
		 			ret_6 = fork ();
		 			
		 			// код третьего процесса (3)
		 			if (ret_6 != 0 && ret_6 != -1) {

		 				waitpid(ret_5, 0, 0);
		 				waitpid(ret_6, 0, 0);
		 				exit(EXIT_SUCCESS);
		 			}
		 				// код шестого процесса (6)
		 				else if (ret_6 == 0) {
		 					printf ("It's procces(6)! PPID = %d, PID = %d\n\n",\
		 							getppid(), getpid());
		 					exit(EXIT_SUCCESS);
		 				}
		 					// ошибка при порождении шестого процесса
		 					else if (ret_6 == -1) {
		 						perror (" Fork err (3)->(6) ");
								exit (EXIT_FAILURE);
		 					}
		 		}
		 			// находимся в пятом процессе (5)
		 			else if (ret_5 == 0) {
		 				printf ("It's procces(5)! PPID = %d, PID = %d\n\n",\
		 						getppid(), getpid());
						exit(EXIT_SUCCESS);
		 			}
		 				// ошибка при порождении пятого процесса
		 				else if (ret_5 == -1) {
		 					perror (" Fork err (3)->(5) ");
							exit (EXIT_FAILURE);
		 				}

			}
				// ошибка при порождении третьего процесса
				else if (ret_3 == -1) {
					perror (" Fork err (1)->(3) ");
					exit (EXIT_FAILURE);
				}
	}	
		// код второго процесса (2)
		else if (ret_2 == 0) {
			printf ("It's procces(2)! PPID = %d, PID = %d\n\n",\
		 			getppid(), getpid());

			// порождаем четвертый процесс (2)->(4)
			ret_4 = fork ();

			if (ret_4 != 0 && ret_4 != -1) {

			}
				// код четвертого процесса (4)
				else if (ret_4 == 0) {
					printf ("It's procces(4)! PPID = %d, PID = %d\n\n",\
		 			getppid(), getpid());
		 			exit(EXIT_SUCCESS);
				}
					// ошибка при порождении четвертого процесса
					else if (ret_4 == -1) {
						perror (" Fork err (2)->(4) ");
						exit (EXIT_FAILURE);
					}

			waitpid(ret_4, 0, 0);
			exit(EXIT_SUCCESS);
		}
			// ошибка при порождении второго процесса
			else if (ret_2 == -1) {
				perror (" Fork err (1)->(2) ");
				exit (EXIT_FAILURE);
			}
	
	waitpid(ret_2, 0, 0);
	waitpid(ret_3, 0, 0);
	exit(EXIT_SUCCESS);
}