#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define BUFFER_SIZE 10240
using namespace std;
typedef struct{
  int seq_num;
  int ack_num;
  int rwnd;
}send_pkt;
typedef struct{
  int seq_num;
  int ack_num;
  char _data[BUFFER_SIZE];
}rec_pkt;
class TCP{
  public:
    TCP();
    ~TCP();
    void ini();
    void get_seq_num();
    void set_zero();
    void server(char *ip,char *C_PORT_NUM);
    void three_way(char *_ip,char *_port);
    void data_trans();
  private:
    int port,myfd,seq_num,ack,n_seq_num,rwnd;
    char result[BUFFER_SIZE];
    struct sockaddr_in myaddr,serveraddr;
    struct hostent *hp;
    send_pkt _send;
    rec_pkt _rec;
};
TCP::TCP()
{
  port = 10260;
}
TCP::~TCP()
{}
void TCP::ini()
{
  myfd = socket(AF_INET,SOCK_STREAM,0);
  bzero((char *)&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(port);
  if(bind(myfd,(struct sockaddr*)&myaddr,sizeof(myaddr)) < 0)
  {
    cout<<"Bind error."<<endl;
    exit(1);
  }
}
void TCP::get_seq_num()
{
  srand((int)time(NULL));
  seq_num = rand() % 10000 + 1;
}
void TCP::set_zero()
{
  memset(_rec._data,0,100);
}
void TCP::server(char *ip,char *C_PORT_NUM)
{
  bzero(&serveraddr,sizeof(serveraddr));
  serveraddr.sin_family = PF_INET;
  hp = gethostbyname(ip);
  if (hp == 0) 
  {
    cout<<"IP error"<<endl;
    exit(1);
  }
  bcopy(hp->h_addr_list[0],(caddr_t)&serveraddr.sin_addr,hp->h_length);
  int PORT_NUM = atoi(C_PORT_NUM);
  serveraddr.sin_port = htons(PORT_NUM);
  connect(myfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
}
  void TCP::three_way(char *_ip,char *_port)
{
  cout<<"=====Start the three-way handshake====="<<endl;
  _send.seq_num = seq_num;
  _send.ack_num = 0;
  send(myfd,&_send,sizeof(_send),0);
  cout<<"Send a packet(SYN/ACK) to "<<_ip<<" : "<<_port<<endl;
  int rec = recv(myfd,&_rec,sizeof(_rec),0);
  cout<<"Receive a packet(SYN/ACK) from "<<_ip<<" : "<<_port<<endl;
  cout<<"       Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
  _send.seq_num = _rec.ack_num;
  _send.ack_num = _rec.seq_num + 1;
  send(myfd,&_send,sizeof(_send),0);
  cout<<"Send a packet(ACK) to "<<_ip<<" : "<<_port<<endl;
  cout<<"=====Complete the three-way handshake====="<<endl;
}
void TCP::data_trans()
{
  int rec,mark = 0,len = 0,LBRead,LBRcvd;
  rwnd = 10240;
  while(mark <= BUFFER_SIZE)
  {
    rec = recv(myfd,&_rec,sizeof(_rec),0);
    cout<<"Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
    _send.ack_num = _rec.seq_num * 2;
    _send.seq_num = _rec.ack_num;
    for(int i = 0;i < BUFFER_SIZE;i++)
    {
      if(_rec._data[i] != 0)
        len ++;
      else if(_rec._data[i] == 0)
        break;
    }
    for(int i = mark;i < mark+len;i++)
      result[i] = _rec._data[i-mark];
    mark += len;
    rwnd = BUFFER_SIZE - len;
    _send.rwnd = rwnd;
    send(myfd,&_send,sizeof(_send),0);
  }
}
int main(int argc, char**argv)
{
  TCP myTCP;
  if(argc != 3)
  {
    cout<<"./client1 server_ip server_port"<<endl;
    exit(1);
  }
  myTCP.set_zero();
  myTCP.get_seq_num();
  myTCP.ini();
  myTCP.server(argv[1],argv[2]);
  myTCP.three_way(argv[1],argv[2]);
  myTCP.data_trans();
}
