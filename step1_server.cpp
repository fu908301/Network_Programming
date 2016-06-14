#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define BUFFER_SIZE 10240
using namespace std;
typedef struct{
  int seq_num;
  int ack_num;
}send_pkt;
typedef struct{
  int seq_num;
  int ack_num;
}rec_pkt;
class TCP{
  public:
    TCP();
    ~TCP();
    void getip();
    void print_parameters();
    void create_data();
    void get_seq_num();
    void open_socket();
    void check_bind();
    void check_listen();
    void go_listen();
    void three_way();
   // void chartoint();
    //void inttochar();
  private:
    int ipfd,RTT,MSS,threshold,port,n_seq_num,listenfd,clientfd,ack,seq_num;
    char _data[BUFFER_SIZE],synack[10],syn[5],c_ack[5];
    struct sockaddr_in *addr,seraddr,cliaddr;
    struct ifreq if_info;
    send_pkt _send;
    rec_pkt _rec,_rec2;
    char ip[INET_ADDRSTRLEN],cip[INET_ADDRSTRLEN];
};
TCP::TCP()
{
  RTT = 200;
  MSS = 512;
  threshold = 65535;
  port = 10250;
}
TCP::~TCP()
{}
void TCP::getip()//get the server ip
{
  ipfd = socket(AF_INET, SOCK_DGRAM, 0);
  strcpy(if_info.ifr_name, "eth0");
  ioctl(ipfd, SIOCGIFADDR, &if_info);
  addr = (struct sockaddr_in *)&if_info.ifr_addr;
  inet_ntop(AF_INET,&addr->sin_addr,ip,sizeof(ip));
}
void TCP::print_parameters()
{
  cout<<"====Parameter===="<<endl;
  cout<<"The RTT delay = "<<RTT<<" ms"<<endl;
  cout<<"The threshold = "<<threshold<<" bytes"<<endl;
  cout<<"The MSS = "<<MSS<<" bytes"<<endl;
  cout<<"The buffer size = "<<BUFFER_SIZE<<" bytes"<<endl;
  cout<<"Sever's IP is "<<ip<<endl;
  cout<<"Sever is listening on port "<<port<<endl;
  cout<<"==============="<<endl;
}
void TCP::create_data()//create the data randomly.
{
  int mark;
  char char_table[62];
  srand((int)time(NULL));
  for(int i = 0;i < 26;i++)
    char_table[i] = i + 65;
  for(int i = 26;i < 52;i++)
    char_table[i] = i + 71;
  for(int i = 52;i < 62;i++)
    char_table[i] = i - 4;
  for(int i = 0;i < BUFFER_SIZE;i++)
  {
    mark = rand()%62;
    _data[i] = char_table[mark];
  }
}
void TCP::get_seq_num()
{
  seq_num = rand() % 10000 + 1;
}
void TCP::open_socket()
{
  listenfd = socket(PF_INET,SOCK_STREAM,0);
  if(listenfd < 0)
  {
    cout<<"Create socket error."<<endl;
    exit(1);
  }
  bzero(&seraddr,sizeof(seraddr));
  seraddr.sin_family = AF_INET;
  seraddr.sin_port = htons(port);
  seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
}
void TCP::check_bind()
{
  if(bind(listenfd,(struct sockaddr*)&seraddr,sizeof(seraddr)) < 0)
  {
    cout<<"Bind error."<<endl;
    exit(1);
  }
}
void TCP::check_listen()
{
  if(listen(listenfd,10) < 0)
  {
    cout<<"Listen error."<<endl;
    exit(1);
  }
}
void TCP::go_listen()
{
  while(1)
  {
    socklen_t addrlen = sizeof(cliaddr);
    cout<<"Listening for client..."<<endl;
    clientfd = accept(listenfd,(struct sockaddr*)&cliaddr,&addrlen);
    if(clientfd < 0)
    {
      cout<<"Accept error."<<endl;
      exit(1);
    }
    three_way();
  }
}
void TCP::three_way()
{
  cout<<"=====Start the three-way handshake====="<<endl;
  int rec = recv(clientfd,&_rec,sizeof(_rec),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr, cip, sizeof(cip));
  cout<<"Receive a packet(SYN) from "<<cip<<" : "<<cliaddr.sin_port<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  _send.seq_num = seq_num;
  _send.ack_num = _rec.seq_num + 1;
  send(clientfd,&_send,sizeof(_send),0);
  cout<<"Send a packet(SYN/ACK) to "<<cip<<" : "<<cliaddr.sin_port<<endl;
  int rec2 = recv(clientfd,&_rec2,sizeof(_rec2),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr,cip,sizeof(cip));
  cout<<"Receive a packet(ACK) from "<<cip<<" : "<<cliaddr.sin_port<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec2.seq_num<<", ack_num = "<<_rec2.ack_num<<")"<<endl;
  cout<<"=====Complete the three-way handshake====="<<endl;
}
/*void TCP::chartoint()
{
  for(int i = 0;i < 5;i++)
    syn[i] = synack[i];
  for(int i = 0;i < 5;i++)
    c_ack[i] = synack[i+5];
  n_seq_num = atoi(syn);
  ack = atoi(c_ack);
}
void TCP::inttochar()
{
  int temp = 4;
  for(int i = 0;i < 5;i++)
  {
    syn[i] = '0';
    c_ack[i] = '0';
  }
  while(seq_num >= 1)
  {
    syn[temp] = seq_num % 10 + 48;
    seq_num /= 10;
    temp --;
  }
  temp = 4;
  while(ack >= 1)
  {
    c_ack[temp] = ack % 10 + 48;
    ack /= 10;
    temp --;
  }
  for(int i = 0;i < 5; i++)
    synack[i] = syn[i];
  for(int i = 0;i < 5; i++)
    synack[i+5] = c_ack[i];
}*/
int main()
{
  TCP myTCP;
  myTCP.getip();
  myTCP.print_parameters();
  myTCP.create_data();
  myTCP.get_seq_num();
  myTCP.open_socket();
  myTCP.check_bind();
  myTCP.check_listen();
  myTCP.go_listen();
}
