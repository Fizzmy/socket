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
int main()
{
	WSADATA wsaData;
	int fixedlen;
	printf("Please input the fixedlen : ");
	scanf("%d",&fixedlen);getchar();
	fixedlen++;
	SOCKET ConnectSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	char sendbuf[DEFAULT_BUFLEN];
	memset(sendbuf,0,sizeof(sendbuf));
	char recvbuf[DEFAULT_BUFLEN];
	memset(recvbuf,0,sizeof(recvbuf)); 
	char ans[DEFAULT_BUFLEN];
	memset(ans,0,sizeof(ans));
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
	//�������ӷ�������ַ��ֱ���ɹ� 
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
	while (1)
	{
		fgets(sendbuf,DEFAULT_BUFLEN,stdin);//����һ�����ݣ����س��� 
		int pos=strlen(sendbuf);
		if (pos==5&&sendbuf[0]=='q'&&sendbuf[1]=='u'&&sendbuf[2]=='i'&&sendbuf[3]=='t'&&sendbuf[4]=='\n')//�ж��ǲ����˳����� 
		{
			iResult=shutdown(ConnectSocket,SD_SEND);//֪ͨ���������ٷ����µ����� 
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
			//������������������Ϣ 
			if (pos!=fixedlen) {printf("too long or too short data!\n");continue;}
			iResult=send(ConnectSocket,sendbuf,pos,0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
		}
		char *recvBuf=recvbuf;
		iResult=recvn(ConnectSocket,recvBuf,fixedlen+5);//��ȡһ�з��������ص����ݣ�+5����Ϊ������echo:�ĳ��� 
		if (iResult>0)
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
		}
		else if (iResult==0)//���������ٷ����µ����� 
		{
			//�ر��׽��֣��ͷ���Դ 
			printf("Connection closed.\n");
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}
		else//���ճ��� 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}
	}
	
	
}
