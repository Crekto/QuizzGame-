all:
	gcc server.c -o server -lsqlite3 -pthread
	gcc client.c -o client