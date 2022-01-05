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
int recvn(SOCKET s,char *recvbuf,int fixedlen)//从套接字s读取长度为fixedlen的数据，放入recvbuf中 
{
	int iResult;
	int cnt;
	cnt=fixedlen;
	while (cnt>0)
	{
		iResult=recv(s,recvbuf,cnt,0);//接收长度为cnt的数据 
		if (iResult<=0) return iResult;//没读入/读入错误直接返回0或-1 
		recvbuf+=iResult;//字符指针对应后移 
		cnt-=iResult;//更改还需接收的数据长度 
	}
	return fixedlen;//返回接收到的总数据长度 
}
int main()
{
	WSADATA wsaData;
	int fixedlen;
	printf("Please input the fixedlen : ");
	scanf("%d",&fixedlen);fixedlen++;//读取定长，这里定长要+1因为要考虑回车符 
	SOCKET ListenSocket=INVALID_SOCKET;
	SOCKET ClientSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
	char recvbuf[DEFAULT_BUFLEN]; 
	char echo[]="ECHO:";
	memset(recvbuf,0,sizeof(recvbuf));
	//memset(ans,0,sizeof(ans));
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
			iResult=recvn(ClientSocket,recvBuf,fixedlen);//接收定长数据 
			if (iResult>0)//接受成功，输出接收结果 
			{
				recvbuf[iResult]='\0';
				fputs(recvbuf,stdout);
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
				printf("recv failed with error:%d\n",WSAGetLastError());
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
			//cout<<strlen(echo)+iResult<<endl;
			iSendResult=send(ClientSocket,recvbuf,iResult,0); //发送收到的数据 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
		}
	}
}
