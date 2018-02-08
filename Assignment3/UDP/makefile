all:	server client
server: udpserver.c;
	gcc udpserver.c -o server -lssl -lcrypto;
client:	udpclient.c;
	gcc udpclient.c -o client -lssl -lcrypto;
clean:	;
	rm client server;
