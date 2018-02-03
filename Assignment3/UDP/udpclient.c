/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h>
#include <openssl/md5.h>
#define BUFSIZE 1024


/* 
 * error - wrapper for perror
 */
void error(char *msg) 
{
    perror(msg);
    exit(0);
}

typedef union
{
    int no;
    char bytes[4];

} int_to_char;


int main(int argc, char **argv) 
{
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) 
    {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(-1);
    }

    /* build the server's Internet address */
    memset((char *) &serveraddr,0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    // bzero(buf, BUFSIZE);
    // printf("Please enter msg: ");
    // fgets(buf, BUFSIZE, stdin);

    ///////////////////////////////////

    serverlen = sizeof(serveraddr);
    

    char hello_message[3*BUFSIZE],hello[BUFSIZE];
    int filesize;
    strcpy(hello,"hello\0");



    memset(buf,'\0',sizeof(buf));


    printf("Please enter the file name: ");
    

    scanf("%s",buf);

    char *filename[1000];
    strcpy(filename,buf);

    struct stat st;
    stat(buf, &st);
    filesize = st.st_size;
    FILE* fp = fopen(buf, "rb");

    sprintf(hello_message,"%s,%s,%d",hello,buf,filesize);

    memset(buf,'\0',sizeof(buf));
    

    struct timeval tv;

    tv.tv_sec = 1; 
    tv.tv_usec = 0;

    
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        printf("Cannot Set SO_RCVTIMEO for socket\n");


    // if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    //     printf("Cannot Set SO_SNDTIMEO for socket\n");
    

    while(1)
    {

        if( sendto (sockfd, hello_message, strlen(hello_message), 0, &serveraddr, serverlen) < 0 )
            error("ERROR in hello");
        printf("waiting for hello_ACK\n");
        if(recvfrom(sockfd, buf, sizeof(buf)-1,0,&serveraddr, &serverlen) < 0)
        {

            error("ERROR in hello ACK");
        }

        printf("\n received something , %s \t ",buf);
        for(int k=0;k<10;k++)
        {
            printf("%c ,",buf[k]);
        }
        printf("\n");

        if(strcmp(buf,"hello_ACK") == 0)
        {
            printf("\n hello ack received \n" );
            break;
        }
 
    }

    int  remain_data = filesize,sent_bytes;

    // if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        // printf("Cannot Set SO_SNDTIMEO for socket\n");

    int seq_number = 1;

    unsigned char packet_buf[BUFSIZE]={0};
    int_to_char char_num;


    memset(packet_buf,0,sizeof(packet_buf));
    int nread = fread(packet_buf+8,1,BUFSIZE-8,fp);       

    while(1)
        {
            // unsigned char buff[1024]={0};
            if(nread > 0)
            {
                //// writing the sequence number

                char_num.no=seq_number;
                packet_buf[0]=char_num.bytes[0];
                packet_buf[1]=char_num.bytes[1];
                packet_buf[2]=char_num.bytes[2];
                packet_buf[3]=char_num.bytes[3];
                
                ///// writing the number of bytes


                char_num.no=nread;
                packet_buf[4]=char_num.bytes[0];
                packet_buf[5]=char_num.bytes[1];
                packet_buf[6]=char_num.bytes[2];
                packet_buf[7]=char_num.bytes[3];
                

                ///// sending the message
                printf("sending the packet with seq_number = %d and sent bytes = %d \n",seq_number,nread);
                if(sendto (sockfd, packet_buf, BUFSIZE , 0, &serveraddr, serverlen) < 0 )
                {
                    printf("ERROR on sending packet with seq number = %d",seq_number);
                    exit(-1);
                }
            }

            ///// listening for the ACK

            memset(buf,'\0',sizeof(buf));
            n = recvfrom(sockfd, buf, sizeof(buf)-1, 0, &serveraddr, &serverlen);

            if(n < 0)
            {
                if (errno == EWOULDBLOCK) 
                {
                    fprintf(stderr, "socket timeout\n");
                    continue;
                }
                else
                {
                    printf("ERROR ACK recvfrom error at seq_number = %d",seq_number);
                    exit(-1);
                }
            }
            else
            {
                char* tokens;
                tokens = strtok(buf,",");
                int i=0;
                char code[10];
                char seq_string[BUFSIZE];
                while (tokens != NULL && i<=1)
                {

                    if(i==0)
                    {
                        // printf("code decoded \t");
                        strcpy(code,tokens);
                    }
                      
                    else if(i==1)
                    {
                        // printf("sequence number decoded\t" );
                        strcpy(seq_string,tokens);
                    }

                    tokens = strtok (NULL, ",");
                    i++;
                }

                int ack_seq_num = atoi(seq_string);
                if(strcmp(code,"ACK")==0 && ack_seq_num == seq_number)
                {
                    printf("\nACK for seq_number= %d received \n",seq_number);
                    seq_number++;
                    remain_data-=nread;
                    memset(packet_buf,0,sizeof(packet_buf));
                    nread = fread(packet_buf+8,1,BUFSIZE-8,fp);       
                }
                else
                {
                    printf("ACK not received for sequence number= %d , continuing \n",seq_number);
                    continue;
                }
            }

            if (remain_data==0)
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



    ///////////////////////////////////
    /* send the message to the server */
    // n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    // if (n < 0) 
      // error("ERROR in sendto");
    
    /* print the server's reply */
    // n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    // if (n < 0) 
      // error("ERROR in recvfrom");
    // printf("Echo from server: %s", buf);

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
    n=recvfrom(sockfd,received_checksum,sizeof(received_checksum),0,&serveraddr,&serverlen);
    if(n<0)
        error("ERROR in receiving the checksum \n");
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
