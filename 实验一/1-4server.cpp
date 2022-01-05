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
int recvn(SOCKET s,char *recvbuf,int fixedlen)
{
	int iResult;
	int cnt;
	cnt=fixedlen;
	while (cnt>0)
	{
		iResult=recv(s,recvbuf,cnt,0);
		if (iResult<=0) return iResult;
		recvbuf+=iResult;
		cnt-=iResult;
	}
	return fixedlen;
}
int recvvl(SOCKET s,char *recvbuf,unsigned int recvbuflen)
{
	int iResult;
	unsigned int reclen;
	iResult=recvn(s,(char *)&reclen,sizeof(unsigned int));
	if (iResult!=sizeof(unsigned int)) return iResult;
	reclen=ntohl(reclen);
	if (reclen>recvbuflen)
	{
		while (reclen>0)
		{
			iResult=recvn(s,recvbuf,recvbuflen);
			if (iResult!=recvbuflen)
			{
				if (iResult<=0) return iResult;
			}
			reclen-=recvbuflen;
			recvbuflen=min(recvbuflen,reclen);
		}
		return -2;
	}
	iResult=recvn(s,recvbuf,reclen);
	return iResult;
}
int main()
{
	WSADATA wsaData;
	SOCKET ListenSocket=INVALID_SOCKET;
	SOCKET ClientSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
	char recvbuf[DEFAULT_BUFLEN+1]; 
	char echo[]="ECHO:";
	char error[]="ERROR:too big data!\n";
	memset(recvbuf,0,sizeof(recvbuf));
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
		while (1)//不断接受数据 
		{
			char *recvBuf=recvbuf;
			iResult=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);//接收变长数据 
			if (iResult>0)//接受成功，输出接收结果 
			{
				recvbuf[iResult]='\0';
				fputs(recvbuf,stdout);
				iResult+=strlen(echo);
				unsigned int sendHead=htonl(iResult);//将消息长度转化成网络字节顺序 
				iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);//作为消息头发送 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
				iSendResult=send(ClientSocket,echo,strlen(echo),0); //发送echo: 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
				iSendResult=send(ClientSocket,recvbuf,iResult-strlen(echo),0); //发送接收到的数据 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
			}
			else if (iResult==0)//客户端不再发送新的数据 
			{
				printf("Connection closed.\n");
				//通知客户端服务器不再发送新的数据
				iResult=shutdown(ClientSocket,SD_SEND);
				if (iResult==SOCKET_ERROR)
				{
					printf("shutdown failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
				//关闭套接字
				closesocket(ClientSocket);
				break;
			}
			else//接收出错 
			{
				if (iResult==-2)//数据包大小超过缓冲区 
				{
					printf("too big data!\n");//输出错误信息 
					unsigned int sendHead=htonl(strlen(error));//向客户端发送错误信息 
					iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);
					if (iSendResult==SOCKET_ERROR)
					{
						printf("send failed with error:%d\n",WSAGetLastError());
						closesocket(ClientSocket);
						break;
					}
					iSendResult=send(ClientSocket,error,strlen(error),0); 
					if (iSendResult==SOCKET_ERROR)
					{
						printf("send failed with error:%d\n",WSAGetLastError());
						closesocket(ClientSocket);
						break;
					}
					continue;//继续接受 
				}
				printf("recv failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
				//关闭套接字，退出循环 
			}
		}
	}
}
