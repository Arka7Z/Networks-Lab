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
#include <openssl/md5.h>
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
    
    /* get File name from the user */
    printf("Please File name: ");
    bzero(buf, BUFSIZE);
    scanf("%s",buf);
    struct stat st;
    stat(buf, &st);
    filesize = st.st_size;
    FILE* fp = fopen(buf, "rb");

    sprintf(Hello_Message,"%s,%s,%d",hello,buf,filesize);
    n=send(sockfd,Hello_Message,sizeof(Hello_Message),0);
    if (n < 0)
      error("ERROR writing to socket");
    int  remain_data = filesize,sent_bytes;
    off_t offset=NULL;
          /* Sending file data */
    
    char server_response[BUFSIZE];
    n=recv(sockfd,server_response,sizeof(server_response),0);
    printf("Server response: %s\n",server_response );

    int read_bytes;

    while(1)
        {
            /* First read file in chunks of BUFSIZE bytes */
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);       

            /* If read was success, send data. */
            if(nread > 0)
            {
                
                write(sockfd, buff, nread);
            }
            if (nread < 1024)
            {
                if (feof(fp))
                    {
                                printf("File Sent\n");
                    
                    }
                            if (ferror(fp))
                                printf("Error Sending file\n");
                            break;
            }
        }
    bzero(server_response, BUFSIZE);

    fclose(fp);
    int fd=open(buf, "r");

    MD5_CTX c;
    char buffer[BUFSIZE];
    ssize_t bytes;
    char out[MD5_DIGEST_LENGTH];

    MD5_Init(&c);
    bytes=read(fd, buffer, BUFSIZE);
    while(bytes > 0)
    {
        MD5_Update(&c, buffer, bytes);
        bytes=read(fd, buffer, BUFSIZE);
    }

    MD5_Final(out, &c);

    close(fd);
    char received_checksum[MD5_DIGEST_LENGTH];
    n=recv(sockfd,received_checksum,sizeof(received_checksum),0);
    int same=1;
    int i;
    for(i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        if(received_checksum[i]!=(out[i]))
            same=0;
    }
    if(same==1)
    {
        printf("MD5 Matched\n");
    }
    else
    {
        printf("MD5 Not Matched\n");
    }
    close(fd);
    close(sockfd);

    return 0;
}
