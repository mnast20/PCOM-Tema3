CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c parson/parson.c
	$(CC) -o client client.c requests.c helpers.c buffer.c parson/parson.c -Wall

run: client
	./client

clean:
	rm -f *.o client
