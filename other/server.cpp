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
#include<fstream>
#include<cmath>
#define BUFFER_SIZE 32468
using namespace std;
typedef struct{
  int seq_num;
  int ack_num;
  char buf[BUFFER_SIZE];
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
    void get_data();
    void get_seq_num();
    void open_socket();
    void check_bind();
    void check_listen();
    void go_listen();
    void data_trans();

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
  MSS = 1024;
  threshold = 65535;
  port = 7777;
  c_port = 23456;
 
}
TCP::~TCP()
{}
void TCP::set_zero()
{
  memset(_send.buf,0,BUFFER_SIZE);
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
void TCP::get_data()//create the data randomly.
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
	cout << "connect successfully\n";
	//fork();
	data_trans();
	close(clientfd);
  }
  close(listenfd);
}
void TCP::data_trans()
{
  _rec.rwnd = 10240;
  int cwnd = 1;
  int mark = 0;
  int len;
  int loc = 1;
	_send.seq_num = 1;
	
	ifstream file1;
	ifstream f1;
	ofstream fileOut1;
	file1.open("Video1.mp4", ios::in | ios::binary | ios::ate);
	f1.open("Video1.mp4", ios::in | ios::binary);
	//fileOut1.open("out1.mp4", ios::out | ios::binary);
	cout << "file size : " << file1.tellg() << endl;
	int chunkNum = ceil((float)file1.tellg()/32468);	
	cout << chunkNum << endl;
	int fileSize = file1.tellg();
	cout<<"Start to send the file, the file size is "<< fileSize << " bytes."<<endl;
	//send fileSize to client
	send(clientfd,&fileSize,sizeof(fileSize),0);
	//send chunkNum to client
	send(clientfd,&chunkNum,sizeof(chunkNum),0);

	int bufSize = sizeof(_send.buf);
	for(int i=0 ; i<chunkNum ; i++){
		
		if(fileSize > bufSize)	fileSize -= bufSize;
		else bufSize = fileSize;	
		
		f1.read(_send.buf, bufSize);
		
		
		while(mark < bufSize)
  	{
    len = 0;
    if(cwnd > _rec.rwnd)
      cwnd = 1;
    cout<<"cwnd = "<<cwnd<<", rwnd = "<<_rec.rwnd<<", threshold = "<<threshold<<endl;
  
    for(int i = 0;i < bufSize ; i++)
    {
      if(_send.buf[i] != 0)
        len++;
      else if(_send.buf[i] == 0)
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
    cwnd += cwnd;
    if(cwnd > MSS)
      cwnd = MSS;
    set_zero();
    loc += len;
  	}



		//cout << buf;
		//fileOut1.write(buf, bufSize);
	}
	
	file1.close();
	f1.close();
	//fileOut1.close();
	
  

  
  
  
  cout<<"The file transmission is finish."<<endl;
}

int main()
{
	TCP myTCP;
	myTCP.set_zero();
	myTCP.getip();
	myTCP.print_parameters();
	myTCP.get_data();
	myTCP.get_seq_num();
	myTCP.open_socket();
	myTCP.check_bind();
	myTCP.check_listen();
	myTCP.go_listen();
			
  
  return 0;
}
