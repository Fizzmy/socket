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
int recvline(SOCKET s,char *recvbuf)//��s�׽��ֽ���һ�����ݣ��浽recvbuf�� 
{
	int len=0,iResult;
	while (1)
	{
		iResult=recv(s,recvbuf,1,0);//��ȡһ���ַ� 
		if (iResult>0)//�ɹ����� 
		{
			recvbuf++;len++;//�ƶ��ַ�ָ�룬�����ܳ��� 
			if (*(recvbuf-1)=='\n')//���������ǻس��򷵻��ܳ��� 
			{
				*recvbuf='\0';
				return len;
			}
		}
		else return iResult;//û����/�������ֱ�ӷ���0��-1 
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
	//��ʼ���׽��� 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//����TCPЭ����ز��� 
	hints.ai_family=AF_UNSPEC;//����ipv4��ipv6����ͬѡ��
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	hints.ai_flags=AI_PASSIVE; 
	//������������ַ�Ͷ˿ں� 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//���������׽��� 
	ListenSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ListenSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//�������׽��ְ󶨵�ַ�Ͷ˿ں� 
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
	//������������
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
		//�����������󣬷��������׽��� 
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
			iResult=recvline(ClientSocket,recvBuf);//����һ�����ݵ�recvbuf�� 
			if (iResult>0)
			{
				fputs(recvbuf,stdout);//������յ������� 
				iSendResult=send(ClientSocket,recvbuf,iResult,0); //���䷢�ؿͻ��� 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					break;
				}
			} 
			else if (iResult==0)//��⵽�ͻ��˲��ٷ����µ����� 
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
			else//���ճ��� 
			{
				printf("recv failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
		}
	}
}
