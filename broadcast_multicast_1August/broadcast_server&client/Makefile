all: client.out server.out

client.out: client.c
	gcc -o client.out client.c -g

server.out: server.c 
	gcc -o server.out server.c -g

clean:
	-rm client.out server.out