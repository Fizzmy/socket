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
	//����UDPЭ����ز��� 
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
	//�޸��׽ӿڵĹ���ģʽ ����Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ServerSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);//�������ݣ���¼Դ��ַ�Ͷ˿� 
		if (iResult>=0)//���ܳɹ���������ս�� 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			iSendResult=sendto(ServerSocket,recvbuf,iResult,0,(SOCKADDR *)&clientaddr,clientlen); //���ؽ��յ������� 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ServerSocket);
				continue;
			}
		}
		else//���ճ��� 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			continue;
			//����ѭ�� 
		}
	}
	WSACleanup();
}
