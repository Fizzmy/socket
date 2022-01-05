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
	SOCKET ListenSocket=INVALID_SOCKET;
	SOCKET ClientSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
	char recvbuf[DEFAULT_BUFLEN]; 
	char ansTime[DEFAULT_BUFLEN];
	char error[]="ERROR: Invalid command!";
	memset(recvbuf,0,sizeof(recvbuf));
	memset(ansTime,0,sizeof(ansTime));
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
			//通过连接套接字进行通信 
			iResult=recv(ClientSocket,recvbuf,recvbuflen,0);
			if (iResult>0)//接收到了数据 
			{
				memset(recvbuf,0,iResult);
				time_t nowTime = time(NULL);
				//获取本地时间  
				tm*  localTime = localtime(&nowTime);  
				//转换为年月日星期时分秒  
				strcpy(ansTime,asctime(localTime));  
				iSendResult=send(ClientSocket,ansTime,sizeof(ansTime),0); //发送转换结果 
				iResult=shutdown(ClientSocket,SD_SEND);//不再发送新数据 
				if (iResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
			}
			else if (iResult==0)//连接已关闭或客户端不再发送新数据 
			{
				printf("Connection closed!\n");
				//关闭套接字
				closesocket(ClientSocket);
				break;
			}
			else//接收出错 
			{
				printf("recv failed with error:%d\n",WSAGetLastError());//报错信息 
				closesocket(ClientSocket);
				break;
			}	
		}
	}
}
