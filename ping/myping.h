#ifndef _myping_h
#define _myping_h

#include <bits/stdc++.h>
#include <stdio.h>
#include <ctime>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <stdio.h>
#include <time.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<netinet/in.h>
#include<net/ethernet.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <pthread.h>
#include <errno.h>
using namespace std;


#define PACKET_SIZE     4096
#define MAX_WAIT_TIME   6
#define MAX_NO_PACKETS  25

string host_ip="10.117.12.138",dest_string;

char sendpacket[PACKET_SIZE],recvpacket[PACKET_SIZE];
int sockfd, datalen = 56,packets_send = 0, packets_received = 0,receiveExpected=1;
double minrtt=6000.00,maxrtt=0.0,total_RTT=0.0;
struct sockaddr_in dest_addr,from;
pid_t pid;
struct timeval tvrecv;

void set_ip_header();
void send_request(void);
void recv_reply(void);
void compute_stats();
unsigned short calculateChecksum(unsigned short *addr, int len);
int create_packet(int packet_number);
int parse_received_packet(char *buf, int len);
void get_time_difference(struct timeval *out, struct timeval *in);

#endif
