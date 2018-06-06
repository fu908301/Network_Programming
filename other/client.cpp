#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include<fstream>
#define BUFFER_SIZE 32468
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
  int len;
}rec_pkt;
class TCP{
  public:
    TCP();
    ~TCP();
    void ini();
    void get_seq_num();
		void print_parameters();
    void set_zero();
    void server(char *ip,char *C_PORT_NUM);
    void data_trans();
  private:
    int RTT, MSS, port, threshold, c_port, myfd, seq_num, ack, n_seq_num, rwnd;
    char result[BUFFER_SIZE];
    struct sockaddr_in myaddr,serveraddr;
    struct hostent *hp;
    send_pkt _send;
    rec_pkt _rec;
		char ip[INET_ADDRSTRLEN];
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
void TCP::ini()
{
  int yes = 1;
  myfd = socket(AF_INET,SOCK_STREAM,0);
  bzero((char *)&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(c_port);
  if (setsockopt(myfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
  {
    perror("setsockopt");
    exit(1);
  }
  if(bind(myfd,(struct sockaddr*)&myaddr,sizeof(myaddr)) < 0)
  {
    cout<<"Bind error."<<endl;
    exit(1);
  }
}

void TCP::print_parameters()
{
  cout<<"====Parameter===="<<endl;
  cout<<"The RTT delay = "<<RTT<<" ms"<<endl;
  cout<<"The threshold = "<<threshold<<" bytes"<<endl;
  cout<<"The MSS = "<<MSS<<" bytes"<<endl;
  cout<<"The buffer size = "<<BUFFER_SIZE<<" bytes"<<endl;
  cout<<"Client's IP is "<<ip<<endl;
  cout<<"Client is listening on port "<<c_port<<endl;
  cout<<"==============="<<endl;
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

void TCP::data_trans()
{
	int chunkNum, fileSize;
	int bufSize = sizeof(_rec._data);
  int mark = 0,len = 0,_stop = 0;
  rwnd = 10240;
	ofstream fileOut1;
	fileOut1.open("out1.mp4", ios::out | ios::binary);
	//receive fileSize
	recv(myfd,&fileSize,sizeof(fileSize),0);
	//receive chunkNum
	recv(myfd,&chunkNum,sizeof(chunkNum),0);
	
	for(int i=0 ; i < chunkNum ; i++){


		if(fileSize > bufSize)	fileSize -= bufSize;
		else bufSize = fileSize;	
		

		while(mark <= bufSize)
		{
		  len = 0;
		  recv(myfd,&_rec,sizeof(_rec),0);
			fileOut1.write(_rec._data, bufSize);
		  if(_stop == 0)
		    _stop = _rec.ack_num;
		  cout<<"Receive a packet (seq_num = "<<_rec.seq_num<<", ack_num = "<<_rec.ack_num<<")"<<endl;
		  _send.ack_num = _rec.seq_num + _rec.len;
		  _send.seq_num = _rec.ack_num;
		  for(int i = 0;i < bufSize;i++)
		  {
		    if(_rec._data[i] != 0)
		      len ++;
		    else if(_rec._data[i] == 0)
		      break;
		  }
		  for(int i = mark;i < mark+len;i++)
		    result[i] = _rec._data[i-mark];
		  mark += len;
		  rwnd = bufSize - len;
		  _send.rwnd = rwnd;
		  send(myfd,&_send,sizeof(_send),0);
		  if(_rec.ack_num == _stop + 28)
		  {
		    cout<<"The file transmission is complete."<<endl;
		    break;
		  }
		}
	}
	fileOut1.close();
}

int main(int argc, char *argv[])
{
  TCP myTCP;
	char serverIP[30];
	char serverPort[30];
  if(argc != 2)
  {
    cout<<"./Prog [client port]"<<endl;
    exit(1);
  }
	myTCP.print_parameters();
  myTCP.set_zero();
  myTCP.get_seq_num();
  myTCP.ini();
	cout << "Please Input Server [IP] [Port] you want to connect : \n";
	cin >> serverIP >> serverPort;
  myTCP.server(serverIP, serverPort);
  myTCP.data_trans();

  return 0;
}
