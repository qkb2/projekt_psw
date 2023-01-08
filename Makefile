all: client server

client: inf151825_151823_k.c
	cc -Wall inf151825_151823_k.c -o inf151825_151823_k

server: inf151825_151823_s.c
	cc -Wall inf151825_151823_s.c -o inf151825_151823_s

clean:
	rm -f inf151825_151823_k inf151825_151823_s