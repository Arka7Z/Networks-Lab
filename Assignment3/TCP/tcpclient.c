/*
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0)
      error("ERROR connecting");

    char Hello_Message[3*BUFSIZE],hello[BUFSIZE];
    int filesize;
    strcpy(hello,"Hello\0");
    /* get message line from the user */
    printf("Please File name: ");
    bzero(buf, BUFSIZE);
    scanf("%s",buf);
    //char* fd="/home/cules/Downloads/Socket/Socket/TCP/hello.txt";
    struct stat st;
    stat(buf, &st);
    filesize = st.st_size;
    FILE* fp = fopen(buf, "rb");
    printf("%d filesize\n",filesize );
    sprintf(Hello_Message,"%s,%s,%d",hello,buf,filesize);
    n=send(sockfd,Hello_Message,sizeof(Hello_Message),0);
    if (n < 0)
      error("ERROR writing to socket");
    int  remain_data = filesize,sent_bytes;
    off_t offset=NULL;
          /* Sending file data */
    

    char buffer[BUFSIZE];
    int read_bytes;
    // while( (read_bytes = read(fd, buffer, BUFSIZE)) > 0 )
    //  {
    //      if( (sent_bytes = send(sockfd, buffer, read_bytes, 0))< read_bytes )
    //      {
    //      perror("send error");
    //      return -1;
    //      }
    //
    //  }
    //  fclose(fd);
    // while (((sent_bytes = sendfile(sockfd, fd, &offset, BUFSIZE)) ) && (remain_data > 0))
    //       {
    //               printf("INSIDE for loop client\n" );
    //               fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    //               remain_data -= sent_bytes;
    //
    //               fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    //       }
    /* print the server's reply */

    while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);
            //printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
            if(nread > 0)
            {
                //printf("Sending \n");
                write(sockfd, buff, nread);
            }
            if (nread < 1024)
            {
                if (feof(fp))
            		{
                                printf("End of file\n");
            		
            		}
                            if (ferror(fp))
                                printf("Error reading\n");
                            break;
            }
        }
    bzero(buf, BUFSIZE);

    close(sockfd);
    fclose(fp);
    return 0;
}
