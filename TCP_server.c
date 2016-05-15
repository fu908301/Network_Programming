#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
const int PORT_NUM = 8080;
const int MAX_CONNECT = 10;
int main()
{
  int listenfd,clientfd;
  struct sockaddr_in seraddr,cliaddr;
  char buffer[20] = "Server ACK.";
  char recv_buffer[20];
  listenfd = socket(PF_INET,SOCK_STREAM,0);
  if(listenfd < 0)
  {
    printf("Socket error!\n");
    exit(1);
  }
  bzero(&seraddr,sizeof(seraddr));
  seraddr.sin_family = AF_INET;
  seraddr.sin_port = htons(PORT_NUM);
  seraddr.sin_addr.s_addr = INADDR_ANY;
  if(bind(listenfd,(struct sockaddr*)&seraddr,sizeof(seraddr)) < 0)
  {
    printf("Bind error!\n");
    exit(1);
  }
  if(listen(listenfd,MAX_CONNECT) < 0)
  {
    printf("Listen error!\n");
    exit(1);
  }
  while(1)
  {
    int addrlen = sizeof(cliaddr);
    clientfd = accept(listenfd,(struct sockaddr*)&cliaddr,&addrlen);
    if(clientfd < 0)
    {
      printf("Accept error.\n");
      exit(1);
    }
    int rec = recv(clientfd,recv_buffer,sizeof(recv_buffer),0);
    printf("Received from client: %s, %dbytes\n",recv_buffer,rec);
    send(clientfd,buffer,sizeof(buffer),0);
    close(clientfd);
  }
  close(listenfd);
  return 0;
}
