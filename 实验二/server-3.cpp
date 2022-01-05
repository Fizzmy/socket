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
	//��ʼ���׽��� 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//����TCPЭ����ز��� 
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_flags=AI_PASSIVE; 
	//������������ַ�Ͷ˿ں� 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//����UDP�׽��� 
	ServerSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ServerSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	
	//��UDP�׽��ְ󶨵�ַ�Ͷ˿ں� 
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
	
	int sz=256;
	int t=5;
	int tot=0;
	printf("Size of recvbuf: ");scanf("%d",&sz);//��������С 
	
	//�޸Ļ�������С 
	iResult=setsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sz, sizeof(sz));
	if (iResult<0)
	{
		printf("%d\n",WSAGetLastError());
	}
	//getsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)res, &len);
	printf("Time interval: ");scanf("%d",&t);//���ܼ��
	//���ý��ճ�ʱʱ�� 
	int nTimeOver=4000;
	setsockopt(ServerSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeOver,sizeof(nTimeOver));

	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,sz,0,(SOCKADDR *)&clientaddr,&clientlen);
		if (iResult>=0)//���ܳɹ���������ս�� 
		{
			recvbuf[iResult]='\0';
		//	fputs(recvbuf,stdout);
		//	printf("recv %d bytes.\n",iResult);
			tot++;
		}
		else//���ճ��� 
		{
			//printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ServerSocket);
			break;
		}
		Sleep(t);
	}
	WSACleanup(); 
	printf("%d\n",tot);
	system("pause");
}
