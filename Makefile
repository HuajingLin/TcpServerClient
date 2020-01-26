all:
	gcc server.c -o tcp_server
	gcc client.c -o tcp_client