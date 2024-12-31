CC=gcc

client:
	${CC} client.c winSockHelper.c -o client -lws2_32 

server:
	${CC} server.c winSockHelper.c -ggdb -o server -lws2_32