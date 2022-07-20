all: sig_proccess_one.out sig_proccess_two.out

sig_proccess_one.out: sig_proccess_one.c
	gcc -o sig_proccess_one.out sig_proccess_one.c -g

sig_proccess_two.out: sig_proccess_two.c
	gcc -o sig_proccess_two.out sig_proccess_two.c -g

clean:
	-rm sig_proccess_one.out sig_proccess_two.out