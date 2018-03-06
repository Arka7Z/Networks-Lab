/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/md5.h>
#define BUFSIZE 1024
#define SLEEP_VAL 1
/* 
 * error - wrapper for perror
 */
int w_size =3;
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


static int alarm_fired = 0,alarm_is_on=1;
void mysig(int sig)
{
  pid_t pid;
  printf("*******TIMEOUT********* \n");
  if (sig == SIGALRM && alarm_is_on)
    {
      alarm_fired = 1;                    // FIRE ALARM
      signal(SIGALRM,mysig);
    }
}


int main(int argc, char **argv)
{

  (void) signal(SIGALRM, mysig);

  int sockfd, portno, n;
  int serverlen;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  char *hostname;
  char buf[BUFSIZE];
  int retransmitted = 0;
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
  serverlen = sizeof(serveraddr);


  char hello_message[3*BUFSIZE],hello[BUFSIZE];
  int filesize;
  strcpy(hello,"hello\0");



  memset(buf,'\0',sizeof(buf));


  printf("Please enter the file name: ");


  scanf("%s",buf);

  char filename[1000];
  strcpy(filename,buf);

  struct stat st;
  stat(buf, &st);
  filesize = st.st_size;
  FILE* fp = fopen(buf, "rb");

  sprintf(hello_message,"%s,%s,%d",hello,buf,filesize);

  memset(buf,'\0',sizeof(buf));


  struct timeval tv;

  tv.tv_sec = 1;                       // TIMEOUT IN SECONDS
  tv.tv_usec = 0;                      // DEFAULT


  if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    printf("Cannot Set SO_RCVTIMEO for socket\n");


  while(1)
    {

      if( sendto (sockfd, hello_message, strlen(hello_message), 0, &serveraddr, serverlen) < 0 )
	error("ERROR in hello");
      printf("waiting for hello_ACK\n");
      if(recvfrom(sockfd, buf, sizeof(buf)-1,0,&serveraddr, &serverlen) < 0)
	{
	  error("ERROR in hello ACK");
	}
      printf("\n");

      if(strcmp(buf,"hello_ACK") == 0)
	{
	  printf("\n hello ACK received \n" );
	  break;
	}

    }

  int  remain_data = filesize,sent_bytes;
  int i=0;

  int seq_number = 1;

  unsigned char packet_buf[BUFSIZE]={0};
  int_to_char char_num;

  int nread ;

  int baseptr=0,currptr=1;
  //unsigned char* storage[3];
  unsigned char* storage[w_size];

  //for(i=0;i<3;i++)
  for(i=0;i<w_size;i++)
    {
      storage[i]=(unsigned char*)(malloc(sizeof(char)*BUFSIZE));
      memset(storage[i],0,sizeof(packet_buf));
    }
  int stor_count=0;
  while(1)
    {
      // unsigned char buff[1024]={0};
      if(baseptr<=currptr+w_size)
	{
	  memset(packet_buf,0,sizeof(packet_buf));
	  nread = fread(packet_buf+8,1,BUFSIZE-8,fp);
	  remain_data-=nread;
	  if(nread > 0)                                // SENDING THE DATA
	    {
	      // writing the sequence number

	      char_num.no=baseptr+1;
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


	      // sending the message
	      printf("sending the packet with seq_number = %d and sent bytes = %d \n",baseptr+1,nread);
	      if(sendto (sockfd, packet_buf, BUFSIZE , 0, &serveraddr, serverlen) < 0 )
		{
		  printf("ERROR on sending packet with seq number = %d",baseptr+1);
		  exit(-1);
		}
	      memset(storage[stor_count],0,sizeof(packet_buf));
	      strcpy(storage[stor_count],packet_buf);
	      //stor_count=(stor_count+1)%3;
	      stor_count=(stor_count+1)%w_size;
	      baseptr++;
	    }
	  //if(baseptr- currptr==3)
	  if(baseptr - currptr ==w_size)
	    {
	      // START TIMER
	      alarm_is_on=1;
	      alarm(SLEEP_VAL);
	    }
	}



      // TIMEOUT

      if(alarm_fired)
	{
	  int diff=baseptr-currptr;
	  if(diff==3)
	    i=0;
	  else if(diff==2)
	    i=1;
	  else if(diff==1)
	    i=0;
	  else
	    i=3;
	  //for(;i<3;i++)
	  for(;i<w_size;i++)
	    {
	      memset(packet_buf,0,sizeof(packet_buf));
	      strcpy(packet_buf,storage[i]);

	      // sending the message
	      printf("Retransmitting the packet with seq_number = %d and sent bytes = %d \n",currptr+i,nread);
	      if(sendto (sockfd, packet_buf, BUFSIZE , 0, &serveraddr, serverlen) < 0 )
		{
		  printf("ERROR on sending packet with seq number = %d",seq_number);
		  exit(-1);
		}
	      retransmitted++;
	    }
	  alarm_is_on=1;
	  alarm(SLEEP_VAL);
	}

      // LISTENING FOR THE ACK. Format: %s,%d ACK, Ack number

      memset(buf,'\0',sizeof(buf));
      n = recvfrom(sockfd, buf, sizeof(buf), 0, &serveraddr, &serverlen);

      if(n < 0)
	{
	  if (errno == EWOULDBLOCK)
	    {
	      fprintf(stderr, "socket timeout\n");
	      alarm_fired=1;
	      continue;
	    }
	  else
	    {
	      printf("ERROR in ACK received error at seq_number = %d",seq_number);
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
		  strcpy(code,tokens);
		}

	      else if(i==1)
		{
		  strcpy(seq_string,tokens);
		}

	      tokens = strtok (NULL, ",");
	      i++;
	    }

	  int ack_seq_num = atoi(seq_string);
	  if(strcmp(code,"ACK")==0 && ack_seq_num == baseptr)
	    {
	      currptr=ack_seq_num;
	      alarm_is_on=0;

	    }
	  else if(strcmp(code,"ACK")==0 && ack_seq_num <= baseptr)
	    {
	      currptr=ack_seq_num;
	      alarm_is_on=1;
	      alarm(SLEEP_VAL);
	    }
	  else
	    {
	      continue;
	    }
	}

      if (remain_data==0)
	{
	  if (feof(fp))
	    {
	      printf("File Sent\n");
	      printf("Number of retransmitted packets = %d\n",retransmitted);
	    }
	  if (ferror(fp))
	    printf("Error Sending file\n");
	  break;
	}
    }
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

  for(i=0; i<MD5_DIGEST_LENGTH; i++)
    {
      if(received_checksum[i]!=(out[i]))
	same=1;
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
