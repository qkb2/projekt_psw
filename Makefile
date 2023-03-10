all: client server

client: inf151825_151823_k.c
	cc -Wall -std=c99 inf151825_151823_k.c -o client

server: inf151825_151823_s.c
	cc -Wall -std=c99 inf151825_151823_s.c -o server

clean:
	rm -f client server

cleanup:
	./cleanup.sh

run: client server
	./client
	./server