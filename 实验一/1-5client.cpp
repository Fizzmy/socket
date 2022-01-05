#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<iostream>
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
	SOCKET ConnectSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
	char sendbuf[DEFAULT_BUFLEN+1];
	memset(sendbuf,0,sizeof(sendbuf));
	char recvbuf[DEFAULT_BUFLEN+1];
	memset(recvbuf,0,sizeof(recvbuf)); 
	int iResult;
	int recvbuflen=DEFAULT_BUFLEN;
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置TCP协议相关参数 
	hints.ai_family=AF_UNSPEC;//根据ipv4和ipv6做不同选择
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	//解析服务器地址和端口号 
	char *ip="localhost";
	iResult=getaddrinfo(ip,DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	char ipbuf[16];
	//尝试连接服务器地址，直至成功 
    //创建套接字 
    ConnectSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if (ConnectSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		WSACleanup();
		return 0;
	}
	//向服务器请求连接 
	iResult=connect(ConnectSocket,result->ai_addr,(int)result->ai_addrlen);
	if (iResult==SOCKET_ERROR)
	{
		printf("connect failed with error:%d\n",WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket=INVALID_SOCKET;
		return 0;
	}
	freeaddrinfo(result);
	if (ConnectSocket==INVALID_SOCKET)
    {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 0;
	}
	while (1)
	{
		fgets(sendbuf,DEFAULT_BUFLEN,stdin);
		if (strlen(sendbuf)==5&&sendbuf[0]=='q'&&sendbuf[1]=='u'&&sendbuf[2]=='i'&&sendbuf[3]=='t'&&sendbuf[4]=='\n')
		{
			iResult=shutdown(ConnectSocket,SD_SEND);
			if (iResult==SOCKET_ERROR)
			{
				printf("shutdown failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
		}
		else
		{
			//连接成功后向服务器发送输入的信息 
			unsigned int sendHead=htonl((unsigned int)strlen(sendbuf));
			iResult=send(ConnectSocket,(char *)&sendHead,sizeof(unsigned int),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
			iResult=send(ConnectSocket,sendbuf,(int)strlen(sendbuf),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
		}
		char *recvBuf=recvbuf;
		iResult=recvvl(ConnectSocket,recvBuf,DEFAULT_BUFLEN);
		if (iResult>0)
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
		}
		else if (iResult==0)
		{
			printf("Connection closed.\n");
			//关闭套接字
			break;
		}
		else
		{
			if (iResult==-2)
			{
				printf("too big data!\n");
				continue;
			}
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}
	} 
	//关闭套接字，释放资源 
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}
