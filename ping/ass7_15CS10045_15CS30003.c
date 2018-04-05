#include "myping.h"

void my_handler(int signo)
{
  cout<<endl;
  compute_stats();

}

void get_time_difference(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
         --out->tv_sec;
         out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

void update_min_max_rtt(double rtt)
{
  minrtt=min(minrtt,rtt);
  maxrtt=max(maxrtt,rtt);
}

void compute_stats()
{
    cout<<"----- "<<inet_ntoa(from.sin_addr)<<" "<<"ping statistics"<<"------------------------"<<endl;
    printf("%d packets transmitted, %d received , %.2f percentage lost\n", packets_send,packets_received,(float) ((float)(packets_send - packets_received) /(float) packets_send) * (100.00));
    if(packets_received==0)
    {
        minrtt=0.0;
        total_RTT=0.0;
    }
    printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n", minrtt,(float)((float)total_RTT/(float)packets_received),maxrtt);
    close(sockfd);
    exit(1);

}

void timeout(int signo)
{
    cout<<"Request timeout for icmp_seq "<<packets_send<<endl;
}

void set_ip_header()
{
    struct ip *ip_hdr = (struct ip *) sendpacket;
    ip_hdr->ip_hl = 5;                // 5 words, each word 4bytes->20bytes as shown in ip struct
    ip_hdr->ip_v = 4;
    ip_hdr->ip_id = pid;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_p = IPPROTO_ICMP;
    ip_hdr->ip_ttl = 50;
    ip_hdr->ip_len = 20+8+datalen; // 20 for ip header , 8 icmp header , datalen for the icmp message
    ip_hdr->ip_off = 0;
    inet_pton (AF_INET, host_ip.c_str(), &(ip_hdr->ip_src));        //converts the character string src into a network address structure in the af address family, then copies the network
                                                                    //  address structure to dst. 
    inet_pton (AF_INET, inet_ntoa(dest_addr.sin_addr), &(ip_hdr->ip_dst));    // inet_ntoa -> adress in bytes to ip string
    ip_hdr->ip_sum = calculateChecksum ((unsigned short *) sendpacket, sizeof(struct ip));
}

void send_request()
{
  int packetsize,send_bytes;
    packets_send++;
    packetsize = create_packet(packets_send);
    set_ip_header();
    if ((send_bytes=sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr))) < 0)
    {
        perror("sendto error");
        exit(-1);
      }
}

int create_packet(int packet_number)
{
    int i, packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp = (struct icmp*)(sendpacket+20);
    icmp->icmp_type = ICMP_ECHO;                    // 8
    icmp->icmp_code = 0;                            // 8
    icmp->icmp_cksum = 0;                           // 16
    icmp->icmp_seq = packet_number;                 // 16
    icmp->icmp_id = pid;                           // 16                                                   // used as an identifier
    packsize = 8+datalen;
    tval = (struct timeval*)icmp->icmp_data;         // 16
    gettimeofday(tval, NULL);
    icmp->icmp_cksum = calculateChecksum((unsigned short*)icmp, packsize);
    return packsize+sizeof(struct ip);
}

int parse_received_packet(char *buf, int len)
{

    double rtt;
    struct ip *ip= (struct ip*)buf;
    int ip_hdr_length = ip->ip_hl << 2;

    if (ip->ip_p != IPPROTO_ICMP)
    {
        return -1;
    }

    struct icmp* icmp = (struct icmp*)(buf + ip_hdr_length);

    len -= ip_hdr_length;

    if (len < 8)
    {         cout<<"ICMP packets\'s length is less than 8"<<endl;
        return  - 1;
    }
    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
    {
        struct timeval *tvsend = (struct timeval*)icmp->icmp_data;
        get_time_difference(&tvrecv, tvsend);
        rtt = (double)((double)tvrecv.tv_sec * 1000.00)+(double)((double)tvrecv.tv_usec / 1000.00);
        update_min_max_rtt(rtt);
        total_RTT+=rtt;
        printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n", len,inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
        return 1;
    }
    else
        return  - 1;

}

void recv_packet()
{
    int n;
    socklen_t fromlen;
    extern int errno;
    signal(SIGALRM, timeout);
    fromlen = sizeof(from);
    alarm(MAX_WAIT_TIME);
    if ((n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr*) &from, &fromlen)) < 0)
    {
      if (errno == EINTR)
          return;
      return;
      }
    alarm(0);
    gettimeofday(&tvrecv, NULL);
    if (parse_received_packet(recvpacket, n) ==  - 1)
      return;
    packets_received++;
}

unsigned short calculateChecksum(unsigned short *addr, int len)
{
    int nleft = len, sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;
    while (nleft > 1)
    {
        sum +=  *w++;
        nleft -= 2;
    }
    if (nleft == 1)
    {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;

}

void setup_connection(string dest)
{
  unsigned long inaddr = 0l;
  int size = 50 * 1024;
  struct hostent *host;
  struct protoent *protocol;
  if ((protocol = getprotobyname("icmp")) == NULL){
      perror("getprotobyname");
      exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
  {
      perror("socket error");
      exit(1);
  }
  setuid(getuid()); /* don't need special permissions any more */
  setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
  bzero(&dest_addr, sizeof(dest_addr));
  int one = 1;
  const int *val = &one;
  if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
      cout<<"Cannot set HDRINCL!"<<endl;
  struct timeval timeout;
  timeout.tv_sec = MAX_WAIT_TIME;
  timeout.tv_usec = 0;
  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
      perror("setsockopt failed\n");
  dest_addr.sin_family = AF_INET;
  if (inaddr = inet_addr(dest.c_str()) == INADDR_NONE)
  {
      if ((host = gethostbyname(dest.c_str())) == NULL)
      {
          perror("gethostbyname error");
          exit(1);
      }
      memcpy((char*) &dest_addr.sin_addr, host->h_addr, host->h_length);
  }
  else
      dest_addr.sin_addr.s_addr = inet_addr(dest.c_str());

}

int main(int argc, char *argv[])
{
    signal(SIGINT,my_handler);
    if (argc < 2){
        cout<<"usage:"<<argv[0]<<" hostname/IP address"<<endl;
        exit(1);
    }
    setup_connection(string(argv[1]));
    pid = getpid();
    cout<<"PING "<<argv[1]<<"("<<inet_ntoa(dest_addr.sin_addr)<<"): "<<datalen<<" bytes of data"<<endl;
    while(true)
    {
        send_request();
        recv_packet();
        sleep(1);
        if(packets_send>MAX_NO_PACKETS)
          break;
    }
    compute_stats();
    return 0;

}
