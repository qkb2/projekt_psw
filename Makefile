all: client server

client: inf151825_151823_k.c
	cc -Wall inf151825_151823_k.c -o client

server: inf151825_151823_s.c
	cc -Wall inf151825_151823_s.c -o server

clean:
	rm -f client server

run: client server
	./client
	./server