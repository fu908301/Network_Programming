#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
const int PORT_NUM = 8080;
int main()
{
  int sockfd;
  struct sockaddr_in serveraddr;
  char buffer[20];
  char send_b[20] = "Hi server!";
  sockfd = socket(PF_INET,SOCK_STREAM,0);
  bzero(&serveraddr,sizeof(serveraddr));
  serveraddr.sin_family = PF_INET;
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serveraddr.sin_port = htons(PORT_NUM);
  connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  bzero(buffer,20);
  send(sockfd,send_b,sizeof(send_b),0);
  int rec = recv(sockfd,buffer,sizeof(buffer),0);
  printf("Receivded from server: %s,%d bytes.\n",buffer,rec);
  close(sockfd);
  return 0;
}
