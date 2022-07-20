all: client.out server.out

client.out: client.c
	gcc -o client.out client.c -g -lrt -pthread

server.out: server.c 
	gcc -o server.out server.c -g -lrt -pthread

clean:
	-rm client.out server.out