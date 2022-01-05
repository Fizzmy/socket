#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<iostream>
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
using namespace std; 
int main()
{
	WSADATA wsaData;
	SOCKET ConnectLessSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	char sendbuf[DEFAULT_BUFLEN];
	memset(sendbuf,1,sizeof(sendbuf));
	char recvbuf[DEFAULT_BUFLEN];
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult;
	int recvbuflen=DEFAULT_BUFLEN;
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置UDP协议相关参数
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	//解析服务器地址和端口号 
	char *ip="localhost";
	iResult=getaddrinfo(ip,DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	ConnectLessSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ConnectLessSocket==INVALID_SOCKET)
    {
		printf("socket failed with error: %ld\n",WSAGetLastError());
		WSACleanup();
		return 0;
	}
	int num=0,sz=0,t=0,tot=0;
	printf("Total send num: ");scanf("%d",&num); //发送次数 
	printf("Size of each packge: ");scanf("%d",&sz); //每次发送大小 
	printf("Time interval: ");scanf("%d",&t);//发送间隔 
	for (int i=1;i<=num;i++)
	{
		iResult=sendto(ConnectLessSocket,sendbuf,sz,0,result->ai_addr,(int)result->ai_addrlen);
		if (iResult==SOCKET_ERROR)
		{
			printf("sendto failed with error:%d\n",WSAGetLastError());
			closesocket(ConnectLessSocket);
			WSACleanup();
			return 0;
		}
		else tot++;//printf("send %d bytes.\n",iResult);
		Sleep(t);
	}
	printf("%d\n",tot);
	system("pause");
}
