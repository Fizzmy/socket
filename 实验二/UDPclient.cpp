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
	memset(sendbuf,0,sizeof(sendbuf));
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
	//设置接收超时时间 
	int nTimeOver=1000;
	setsockopt(ConnectLessSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeOver,sizeof(nTimeOver));
	//修改套接口的工作模式 避免Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ConnectLessSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1) 
	{
		fgets(sendbuf,DEFAULT_BUFLEN,stdin);//获取一行数据 
		if (strlen(sendbuf)==5&&sendbuf[0]=='q'&&sendbuf[1]=='u'&&sendbuf[2]=='i'&&sendbuf[3]=='t'&&sendbuf[4]=='\n')//判断是不是quit 
		{
			closesocket(ConnectLessSocket);
			WSACleanup();
			return 0;
		}
		//服务器发送输入的信息 
		iResult=sendto(ConnectLessSocket,sendbuf,(int)strlen(sendbuf),0,result->ai_addr,(int)result->ai_addrlen);
		if (iResult==SOCKET_ERROR)
		{
			printf("sendto failed with error:%d\n",WSAGetLastError());
			closesocket(ConnectLessSocket);
			WSACleanup();
			return 0;
		}
		iResult=recvfrom(ConnectLessSocket,recvbuf,recvbuflen,0,NULL,NULL);//从服务器接收返回的消息 
		if (iResult>=0)//成功输出结果 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
		}
		else
		{
			if (WSAGetLastError()==10060)//超时 
			{
				printf("Time out.\n");
				continue;
			}
			else printf("recv failed with error:%d\n",WSAGetLastError());//其他错误 
			closesocket(ConnectLessSocket);
			WSACleanup();
			return 0;
		}
	}
}
