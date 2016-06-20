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
#include <unistd.h>
#define BUFFER_SIZE 10240
using namespace std;
typedef struct{
  int seq_num;
  int ack_num;
  char _data[BUFFER_SIZE];
  int len;
}send_pkt;
typedef struct{
  int seq_num;
  int ack_num;
  int rwnd;
}rec_pkt;
class TCP{
  public:
    TCP();
    ~TCP();
    void getip();
    void set_zero();
    void print_parameters();
    void create_data();
    void get_seq_num();
    void open_socket();
    void check_bind();
    void check_listen();
    void go_listen();
    void three_way();
    void data_trans();
    void four_way();
  private:
    int ipfd,RTT,MSS,threshold,port,n_seq_num,listenfd,clientfd,ack,seq_num,c_port;
    char _data[BUFFER_SIZE],synack[10],syn[5],c_ack[5];
    struct sockaddr_in *addr,seraddr,cliaddr;
    struct ifreq if_info;
    send_pkt _send;
    rec_pkt _rec;
    char ip[INET_ADDRSTRLEN],cip[INET_ADDRSTRLEN];
};
TCP::TCP()
{
  RTT = 200;
  MSS = 512;
  threshold = 4096;
  port = 10250;
  c_port = 10260;
  for(int i = 0;i < BUFFER_SIZE;i++)
  {
    _send._data[i] = 0;
  }
}
TCP::~TCP()
{}
void TCP::set_zero()
{
  memset(_send._data,0,BUFFER_SIZE);
}
void TCP::getip()//get the server ip
{
  ipfd = socket(AF_INET, SOCK_DGRAM, 0);
  strcpy(if_info.ifr_name, "eth0");
  ioctl(ipfd, SIOCGIFADDR, &if_info);
  addr = (struct sockaddr_in *)&if_info.ifr_addr;
  inet_ntop(AF_INET,&addr->sin_addr,ip,sizeof(ip));
  close(ipfd);
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
  int yes = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
  {
    perror("setsockopt");
    exit(1);
  }
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
  close(listenfd);
}
void TCP::three_way()
{
  cout<<"=====Start the three-way handshake====="<<endl;
  recv(clientfd,&_rec,sizeof(_rec),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr, cip, sizeof(cip));
  cout<<"Receive a packet(SYN) from "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  _send.seq_num = seq_num;
  _send.ack_num = _rec.seq_num + 1;
  send(clientfd,&_send,sizeof(_send),0);
  cout<<"Send a packet(SYN/ACK) to "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  recv(clientfd,&_rec,sizeof(_rec),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr,cip,sizeof(cip));
  cout<<"Receive a packet(ACK) from "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  cout<<"=====Complete the three-way handshake====="<<endl;
  data_trans();
  four_way();
}
void TCP::data_trans()
{
  _rec.rwnd = 10240;
  int cwnd = 1;
  int mark = 0;
  int len;
  int loc = 1;
  int swi = 0;
  int swi2 = 0;
  cout<<"Start to send the file, the file size is 10240 bytes."<<endl;
  cout<<"*****Slow start*****"<<endl;
  _send.seq_num = 1;
  while(mark < BUFFER_SIZE)
  {
    len = 0;
    if(cwnd > _rec.rwnd)
      cwnd = 1;
    cout<<"cwnd = "<<cwnd<<", rwnd = "<<_rec.rwnd<<", threshold = "<<threshold<<endl;
    if(cwnd <= 512)
    {
      for(int i = mark;i < mark + cwnd;i++)
        _send._data[i-mark] = _data[i];
      for(int i = 0;i < BUFFER_SIZE;i++)
      {
        if(_send._data[i] != 0)
         len++;
        else if(_send._data[i] == 0)
          break;
      }
      _send.len = len;
      _send.seq_num = loc;
      _send.ack_num = _rec.seq_num + 1;
      send(clientfd,&_send,sizeof(_send),0);
      cout<<"         Send a packet at : "<<loc<<" byte"<<endl;
      recv(clientfd,&_rec,sizeof(_rec),0);
      cout<<"         Receive a packet (seq_num =  "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
      mark += cwnd;
      loc += len;
    }
    else if(cwnd > 512)
    {
      for(int j = 512;j <= cwnd;j += 512)
      {
        len = 0;
        for(int i = mark;i < mark + 512;i++)
          _send._data[i-mark] = _data[i];
        for(int i = 0;i < BUFFER_SIZE;i++)
        {
          if(_send._data[i] != 0)
          len++;
          else if(_send._data[i] == 0)
            break;
        }
        if(len == 0)
          break;
        _send.len = len;
        _send.seq_num = loc;
        _send.ack_num = _rec.seq_num + 1;
        send(clientfd,&_send,sizeof(_send),0);
        cout<<"         Send a packet at : "<<loc<<" byte"<<endl;
        recv(clientfd,&_rec,sizeof(_rec),0);
        cout<<"         Receive a packet (seq_num =  "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
        mark += 512;
        loc += len;
        if(cwnd == 2048 && swi == 0)
        {
          swi = 1;
          set_zero();
          _send.len = 0;
          _send.seq_num = loc;
          _send.ack_num = _rec.seq_num;
          send(clientfd,&_send,sizeof(_send),0);
          cout<<"         Send a packet at : "<<loc<<" byte"<<endl;
          recv(clientfd,&_rec,sizeof(_rec),0);
          cout<<"         Receive a packet (seq_num =  "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
          cout<<"***Data loss at byte : "<<loc<<endl;
          send(clientfd,&_send,sizeof(_send),0);
          cout<<"         Send a packet at : "<<loc+512<<" byte"<<endl;
          recv(clientfd,&_rec,sizeof(_rec),0);
          cout<<"         Receive a packet (seq_num =  "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
          send(clientfd,&_send,sizeof(_send),0);
          cout<<"         Send a packet at : "<<loc+1024<<" byte"<<endl;
          recv(clientfd,&_rec,sizeof(_rec),0);
          cout<<"         Receive a packet (seq_num =  "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
          cout<<"Receive three duplicate ACKs."<<endl;
          cout<<"*****Fast recovery*****"<<endl;
          cout<<"*****Slow start*****"<<endl;
          threshold = 1024;
          cwnd = cwnd/2;
          break;
        }
        set_zero();
      }
    }
    if(len == 0)
      break;
    if(swi == 1 && swi2 == 0)
      swi2 = 1;
    else if(cwnd >= threshold)
      cwnd += 512;
    else if(cwnd < threshold)
      cwnd += cwnd;
    if(cwnd == threshold)
      cout<<"******Congestion avoidance******"<<endl;
    set_zero();
  }
  cout<<"The file transmission is finish."<<endl;
}
void TCP::four_way()
{
  set_zero();
  cout<<"=====Start four-way handshake.====="<<endl;
  get_seq_num();
  _send.seq_num = seq_num;
  _send.ack_num = _rec.seq_num;
  send(clientfd,&_send,sizeof(_send),0);
  cout<<"Send a packet(FIN) to "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  recv(clientfd,&_rec,sizeof(_rec),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr, cip, sizeof(cip));
  cout<<"Receive a packet(ACK) from "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  recv(clientfd,&_rec,sizeof(_rec),0);
  inet_ntop(AF_INET, &cliaddr.sin_addr, cip, sizeof(cip));
  cout<<"Receive a packet(FIN) from "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  _send.seq_num = _rec.ack_num;
  _send.ack_num = _rec.seq_num + 1;
  send(clientfd,&_send,sizeof(_send),0);
  cout<<"Send a packet (ACK) to "<<cip<<" : "<<ntohs(cliaddr.sin_port)<<endl;
  cout<<"=====Complete the four-way handshake====="<<endl;
  close(clientfd);
}
int main()
{
  TCP myTCP;
  myTCP.set_zero();
  myTCP.getip();
  myTCP.print_parameters();
  myTCP.create_data();
  myTCP.get_seq_num();
  myTCP.open_socket();
  myTCP.check_bind();
  myTCP.check_listen();
  myTCP.go_listen();
  return 0;
}
