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
int main()
{
	WSADATA wsaData;
	SOCKET ConnectSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	char sendbuf[]="g";
	char recvbuf[DEFAULT_BUFLEN];
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult;
	int recvbuflen=DEFAULT_BUFLEN;
	//��ʼ���׽��� 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//����TCPЭ����ز��� 
	hints.ai_family=AF_UNSPEC;//����ipv4��ipv6����ͬѡ��
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	//������������ַ�Ͷ˿ں� 
	char *ip="localhost";
	iResult=getaddrinfo(ip,DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	char ipbuf[16];
    //�����׽��� 
    ConnectSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if (ConnectSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		WSACleanup();
		return 0;
	}
	//��������������� 
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
	//���ӳɹ���������������������Ϣ 
	iResult=send(ConnectSocket,sendbuf,(int)strlen(sendbuf),0);
	if (iResult==SOCKET_ERROR)
	{
		printf("send failed with error:%d\n",WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}
	iResult=shutdown(ConnectSocket,SD_SEND);
	if (iResult==SOCKET_ERROR)
	{
		printf("send failed with error:%d\n",WSAGetLastError());
		closesocket(ConnectSocket);
		return 0;
	}
	while (1)
	{	
		//ѭ�����ܷ��������ص���Ϣ 
		iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
		if (iResult>0)
		{
			//����Ľ����ʽ�ͷ��������� 
			printf("%s",recvbuf);
			memset(recvbuf,0,iResult);
			if (iResult==SOCKET_ERROR)
			{
				printf("shutdown failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
		}
		else if (iResult==0)//���������ݽ�����ϣ��ͻ����Ѿ����ٷ����µ������� 
		{
			printf("Connection closed\n");
			break;
		}
		else
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			break;
		}

	} 
	//�ر��׽��֣��ͷ���Դ 
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}
