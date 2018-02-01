all:	server client
server:	tcpserver.c;gcc tcpserver.c -o server -lssl -lcrypto;
client:	tcpclient.c;gcc tcpclient.c -o client -lssl -lcrypto;
clean:	;rm client server;
