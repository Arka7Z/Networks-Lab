/*
 * tcpserver.c - A simple TCP echo server
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#define BUFSIZE 1024

#if 0
/*
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr;
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

// void compute_checksum(unsigned char out[MD5_DIGEST_LENGTH])
// {
// 	int n;
//     MD5_CTX c;
//     char buf[512];
//     ssize_t bytes;
//     //unsigned char out[MD5_DIGEST_LENGTH];

//     MD5_Init(&c);
//     bytes=read(STDIN_FILENO, buf, 512);
//     while(bytes > 0)
//     {
//         MD5_Update(&c, buf, bytes);
//         bytes=read(STDIN_FILENO, buf, 512);
//     }

//     MD5_Final(out, &c);

// }

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /*
   * listen: make this socket ready to accept connection requests
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");
  printf("Server Running ....\n");
  /*
   * main loop: wait for a connection request, echo input line,
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1)
  {

          /*
           * accept: wait for a connection request
           */
          childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
          if (childfd < 0)
            error("ERROR on accept");

          /*
           * gethostbyaddr: determine who sent the message
           */
          hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,sizeof(clientaddr.sin_addr.s_addr), AF_INET);
          if (hostp == NULL)
            error("ERROR on gethostbyaddr");
          hostaddrp = inet_ntoa(clientaddr.sin_addr);
          if (hostaddrp == NULL)
            error("ERROR on inet_ntoa\n");
          printf("server established connection with %s (%s)\n",hostp->h_name, hostaddrp);


          bzero(buf, BUFSIZE);
          char Message[3*BUFSIZE],code[BUFSIZE],filename[BUFSIZE];
          char filesize_string[BUFSIZE];
          int filesize;
          bzero(Message,3*BUFSIZE);
          bzero(code,BUFSIZE);
          bzero(filename,BUFSIZE);
          bzero(filesize_string,BUFSIZE);
          n=recv(childfd,Message,3*BUFSIZE,0);
          printf("%s\n",Message);

          if (n < 0)
            error("ERROR reading from socket");
          char* pch;
          pch = strtok (Message,",");
          int i=0;
          while (pch != NULL && i<=2)
          {

            if(i==0)
              strcpy(code,pch);
            else if(i==1)
              strcpy(filename,pch);
            else if(i==2)
              strcpy(filesize_string,pch);
            pch = strtok (NULL, ",");
            i++;
          }
          filesize=atoi(filesize_string);
          if(strcmp(code,"Hello\0")==0)                                   // Hello message received
          {
            char acknowledge[BUFSIZE];
            strcpy(acknowledge,"ACK\0");
            n=send(childfd,acknowledge,sizeof(acknowledge),0);
            printf("filesize received: %d\n",filesize );
            FILE *received_file;
            received_file = fopen(filename, "ab");
            int remain_data = filesize,len;
            char buffer[BUFSIZE];

              char recvBuff[1024];
              int bytesReceived;
              while((bytesReceived = read(childfd, recvBuff, 1024)) > 0)
              { 

                  fwrite(recvBuff, 1,bytesReceived,received_file);
                  if(bytesReceived<1024)
                  {	
                  	printf("Server completed receiving the file\n");
                  	break;
                  }
              }
            fclose(received_file);

            int fd=open(filename, "rb");
		    MD5_CTX c;
		    char buf[1024];
		    ssize_t bytes;
		    char out[MD5_DIGEST_LENGTH];

		    MD5_Init(&c);
		    bytes=read(fd, buf, 1024);
		    while(bytes > 0)
		    {
		        MD5_Update(&c, buf, bytes);
		        bytes=read(fd, buf, 1024);
		    }

		    MD5_Final(out, &c);
		    //printf("236 after final out %d\n",MD5_DIGEST_LENGTH);
		    close(fd);

		    n=send(childfd,out,sizeof(out),0);
		    close(fd);
		    close(childfd);

          }
          
  }
}
