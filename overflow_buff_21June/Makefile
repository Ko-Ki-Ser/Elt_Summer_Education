all: pass.out

pass.out: pass.c
	gcc -o pass.out pass.c -fno-stack-protector -no-pie -g


clean:
	-rm pass.out