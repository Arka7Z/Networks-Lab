#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <math.h>

using namespace std;
#define typeof(x) __typeof__(x)
#define BUFSIZE 1024
#define MAXCLIENTS 5
#define traverse(container, it)  for( typeof(container.begin()) it = container.begin(); it != container.end(); it++)
typedef struct {

   char fileName[512];
   int fileSize;
   int noOfChunks;

} fileDetails;


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

typedef struct
{
  char *name;
  char *ip;
  int port_number;
}peer_information;

typedef struct
{
  peer_information user;
  time_t last_activity;
}socket_info;

typedef struct{
  string ip;
  int port_number;
}ip_and_port;

peer_information peers[5];

void error(char *msg)
{
  perror(msg);
  exit(1);
}

void init(map<string, string> &ip_to_name,map<string, ip_and_port> &name_to_info)
{
  peers[0].name = (char *)"arka";
  peers[0].ip = (char *)"10.117.12.138";
  peers[0].port_number = 8085;

  peers[1].name = (char *)"localhost";
  peers[1].ip = (char *)"127.0.0.1";
  peers[1].port_number = 8086;

  peers[2].name = (char *)"aman";
  peers[2].ip = (char *)"10.42.0.180";
  peers[2].port_number = 9000;

  peers[3].name = (char *)"swastik";
  peers[3].ip = (char *)"10.145.194.229";
  peers[3].port_number = 8085;

  peers[4].name = (char *)"server";
  peers[4].ip = (char *)"10.5.18.112";
  peers[4].port_number = 8086;
  for(int i=0;i<5;i++)
  {
    string name=string(peers[i].name);
    string ip=string(peers[i].ip);
    ip_to_name.insert(make_pair(ip,name));
    ip_and_port ip_port;
    ip_port.ip=ip;
    ip_port.port_number=peers[i].port_number;
    name_to_info.insert(make_pair(name,ip_port));
  }
  return;
}

int main(int argc, char **argv)
{
  map<string, string> ip_to_name;
  map<string, ip_and_port> name_to_info;
  map<int, socket_info> present;
  vector<int> currently_active_sockets;
  init(ip_to_name,name_to_info);

  int parentfd; /* parent socket */
  int childfd; /* child socket */
  unsigned int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr,serveraddr1; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */

  int n; /* message byte size */
  fileDetails f;
  fd_set master, read_fds;
  int i,j,max_fd,fd;;
  int client_socket[5]={0};
  /* add stdin and the sock fd to master fd_set */

  /*
   * check command line arguments
   */
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	   exit(1);
  }


  // CREATE PARENT port

  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
    error((char *)"ERROR opening socket");
  int optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
       (const void *)&optval , sizeof(int));
  max_fd = parentfd;
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  int port_number = atoi(argv[1]);
  serveraddr.sin_port = htons((unsigned short)port_number);

  if (bind(parentfd, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0)                 // BIND THE PORT
    error((char *)"ERROR on binding");


  if (listen(parentfd, 5) < 0)                                                                 /* allow 5 requests to queue up */
    error((char *)"ERROR on listen");
  printf("Server Running ....\n");


  clientlen = sizeof(clientaddr);
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(parentfd, &master);
  FD_SET(STDIN_FILENO,&master);



  struct timeval t;
  t.tv_sec = 1;
  t.tv_usec =0;
  while(true)
  {
    read_fds = master;
    if(select(max_fd+1,&read_fds, NULL, NULL, &t)>=0)
    {
      for(i=0;i<=max_fd;i++)
      {
        if(FD_ISSET(i,&read_fds))
        {
                  if(i==parentfd)
                  {
                        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
                        if (childfd < 0)
                          error((char *)"ERROR on accept");
                        getpeername(childfd, (struct sockaddr *)&clientaddr, &clientlen);
                        FD_SET(childfd, &master);                               // Add new socket connection to fd collection

                        socket_info new_connection;
                        new_connection.user.ip = inet_ntoa(clientaddr.sin_addr) ;
                        new_connection.user.port_number = ntohs(clientaddr.sin_port);

                        time(&new_connection.last_activity);

                        //present[childfd] = new_connection;
                        // for(int q=0;q<5;q++)
                        // {
                        //
                        //     if( strcmp(peers[q].ip,new_connection.user.ip)==0 )
                        //     {
                        //         strcpy(new_connection.user.name,peers[q].name);
                        //     }
                        //
                        // }
                        new_connection.user.name=(char*)(malloc(sizeof(100)));
                        new_connection.user.name=(char*)((ip_to_name.find(string(new_connection.user.ip))->second).c_str());
                        present.insert(make_pair(childfd,new_connection));
                        currently_active_sockets.push_back(childfd);
                        cout<<"Creating new connection, ip: "<<inet_ntoa(clientaddr.sin_addr)<<". Port: "<< ntohs(clientaddr.sin_port)<<". Peer name: "<<new_connection.user.name<<endl;
                        max_fd = max(max_fd, childfd);
                        //printf("New connection , socket fd is %d , ip is : %s , port : %d , name:%s\n" , childfd , inet_ntoa(clientaddr.sin_addr) , ntohs(clientaddr.sin_port),new_connection.user.name);

                  }
                  else if(i==STDIN_FILENO)
                  {
                        bzero(buf,BUFSIZE);
                        scanf(" %[^\n]s",buf);

                        char* hostname = strtok(buf,"/");
                        char* msg = strtok(NULL,"\n");
                        int sockfd;
                        char *recv_ip;
                        int recv_port;
                        if(name_to_info.find(string(hostname))==name_to_info.end())
                        {
                          cout<<"Incorrect peer name, please try again"<<endl;
                          continue;
                        }
                        else
                        {
                          ip_and_port ip_port=name_to_info.find(string(hostname))->second;
                          recv_ip=(char*)ip_port.ip.c_str();
                          recv_port=ip_port.port_number;
                        }
                        cout<<"rec "<<recv_ip<< " "<<recv_port<<endl;
                        //map<int, socket_info>::iterator iter;


                        bool socket_present=false;

                        //for(iter = present.begin(); iter != present.end(); iter++)
                        traverse(present,iter)
                        //for(auto const& iter:present)
                        {
                              socket_info soc_temp = iter->second;
                              cout<<"blah "<<soc_temp.user.ip<<" "<<soc_temp.user.port_number <<endl;
                              if(strcmp(soc_temp.user.ip, recv_ip)==0 || soc_temp.user.port_number == recv_port)
                              {
                                    socket_present=true;
                                    sockfd = iter->first;
                                    break;
                              }
                        }
                        if(socket_present)
                        {
                          n = send(sockfd, msg, BUFSIZE, 0);
                          if(n<0)
                            error((char *)"Error in send");
                          time(&(present[sockfd].last_activity));
                        }

                        else
                        {
                              sockfd = socket(AF_INET, SOCK_STREAM, 0);
                              if (sockfd < 0)
                                error((char *)"ERROR opening socket");
                              struct hostent* server = gethostbyname(recv_ip);
                              if (server == NULL)
                              {
                                fprintf(stderr,"ERROR, no such host as %s\n", hostname);
                                exit(0);
                              }
			                     /* build the server's Internet address */
                              bzero((char *) &serveraddr1, sizeof(serveraddr1));
                              serveraddr1.sin_family = AF_INET;
                              bcopy((char *)server->h_addr, (char *)&serveraddr1.sin_addr.s_addr, server->h_length);
                              serveraddr1.sin_port = htons(recv_port);
                              /* connect: create a connection with the server */
                              if (connect(sockfd, (struct sockaddr*)&serveraddr1, sizeof(serveraddr1)) < 0)
                              {
                                error((char *)"ERROR connecting");
                              }

                              socket_info new_connection;
                              new_connection.user.ip = recv_ip;
                              new_connection.user.port_number = recv_port;
                              new_connection.user.name=(char*)(malloc(sizeof(100)));
                              new_connection.user.name=(char*)((ip_to_name.find(string(new_connection.user.ip))->second).c_str());

                              time(&new_connection.last_activity);
                              present.insert(make_pair(sockfd,new_connection));
                              n = send(sockfd, msg, BUFSIZE, 0);
                              if(n<0)
                                error((char *)"Error in send");
                              max_fd = max(sockfd, max_fd);

                              FD_SET(sockfd, &master);
                              currently_active_sockets.push_back(sockfd);

                        }


                  }
                  else
                   {

                        bzero(buf, BUFSIZE);
                        int n_read = recv(i,buf,BUFSIZE,0);
			                  if(n_read==0)
                        {
                          close(i);
                          FD_CLR(i, &master);
                          continue;
                        }
                        if(n_read<0)
                          error((char *)"Error in receive");
                        buf[n_read] = '\0';
                        cout<<present[i].user.name<<": "<<buf<<endl;
                  }
        }
      }
    }
    time_t current_time;
    map<int, socket_info>::iterator it;
    //cout<<"currently_active_sockets: "<<currently_active_sockets.size()<<endl;
    for(int k=0; k< currently_active_sockets.size(); k++)
    {
          time(&current_time);
          if(current_time-present[currently_active_sockets[k]].last_activity>=20)
          {
            if(present.find(currently_active_sockets[k])==present.end())
              cout<<"Time out, closing connection"<<endl;
            else
              cout<<"Time out, closing connection for "<<(present.find(currently_active_sockets[k]))->second.user.name<<endl;
            close(currently_active_sockets[k]);
            FD_CLR(currently_active_sockets[k],&master);
            it = present.find(currently_active_sockets[k]);
            present.erase(it);
            currently_active_sockets.erase(currently_active_sockets.begin()+k);
          }
    }
  }
  return 0;
}
