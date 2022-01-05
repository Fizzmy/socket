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
int recvline(SOCKET s,char *recvbuf)//从s套接字接收一行数据，存到recvbuf里 
{
	int len=0,iResult;
	while (1)
	{
		iResult=recv(s,recvbuf,1,0);//读取一个字符 
		if (iResult>0)//成功读入 
		{
			recvbuf++;len++;//移动字符指针，增加总长度 
			if (*(recvbuf-1)=='\n')//如果读入的是回车则返回总长度 
			{
				*recvbuf='\0';
				return len;
			}
		}
		else return iResult;//没读入/读入错误直接返回0或-1 
	}
} 
int main()
{
	WSADATA wsaData;
	SOCKET ListenSocket=INVALID_SOCKET;
	SOCKET ClientSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
	char recvbuf[DEFAULT_BUFLEN]; 
	char ans[DEFAULT_BUFLEN];
	memset(recvbuf,0,sizeof(recvbuf));
	memset(ans,0,sizeof(ans));
	int iResult,iSendResult;
	int recvbuflen=DEFAULT_BUFLEN;
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置TCP协议相关参数 
	hints.ai_family=AF_UNSPEC;//根据ipv4和ipv6做不同选择
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	hints.ai_flags=AI_PASSIVE; 
	//解析服务器地址和端口号 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//创建监听套接字 
	ListenSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ListenSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//将监听套接字绑定地址和端口号 
	iResult=bind(ListenSocket,result->ai_addr,(int)result->ai_addrlen);
	if (iResult==SOCKET_ERROR)
	{
		printf("bind failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	freeaddrinfo(result);
	//监听连接请求
	iResult=listen(ListenSocket,SOMAXCONN);
	if (iResult==SOCKET_ERROR) 
	{
		printf("listen failed with error:%d\n",WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	} 
	while (1)
	{ 	
		//接受连接请求，返回连接套接字 
		ClientSocket=accept(ListenSocket,NULL,NULL);
		if (ClientSocket==INVALID_SOCKET) 
		{
			printf("accept failed with error:%d\n",WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}
		printf("Connection established!\n");
		while (1)
		{
			char *recvBuf=recvbuf; 
			iResult=recvline(ClientSocket,recvBuf);//接收一行数据到recvbuf中 
			if (iResult>0)
			{
				fputs(recvbuf,stdout);//输出接收到的数据 
				iSendResult=send(ClientSocket,recvbuf,iResult,0); //将其发回客户端 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
			} 
			else if (iResult==0)//检测到客户端不再发送新的数据 
			{
				printf("Connection closed.\n");
				iResult=shutdown(ClientSocket,SD_SEND);
				if (iResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
				closesocket(ClientSocket);
				break;
			}
			else//接收出错 
			{
				printf("recv failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
		}
	}
}
