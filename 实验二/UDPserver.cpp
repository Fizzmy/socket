#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstdio>
#include<iostream>
#include<time.h>
using namespace std;
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
int main()
{
	WSADATA wsaData;
	SOCKET ServerSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	char recvbuf[DEFAULT_BUFLEN+1]; 
	char echo[]="ECHO:";
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult,iSendResult;
	int recvbuflen=DEFAULT_BUFLEN;
	sockaddr_in clientaddr;
	int clientlen=sizeof(sockaddr_in);
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置UDP协议相关参数 
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_flags=AI_PASSIVE; 
	//解析服务器地址和端口号 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//创建UDP套接字 
	ServerSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ServerSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//将UDP套接字绑定地址和端口号 
	iResult=bind(ServerSocket,result->ai_addr,(int)result->ai_addrlen);
	if (iResult==SOCKET_ERROR)
	{
		printf("bind failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ServerSocket);
		WSACleanup();
		return 0;
	}
	freeaddrinfo(result);
	//修改套接口的工作模式 避免Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ServerSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);//接收数据，记录源地址和端口 
		if (iResult>=0)//接受成功，输出接收结果 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			iSendResult=sendto(ServerSocket,recvbuf,iResult,0,(SOCKADDR *)&clientaddr,clientlen); //发回接收到的数据 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ServerSocket);
				continue;
			}
		}
		else//接收出错 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			continue;
			//继续循环 
		}
	}
	WSACleanup();
}
