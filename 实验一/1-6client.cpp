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
	char num1[DEFAULT_BUFLEN+1];
	memset(num1,0,sizeof(num1));
	char num2[DEFAULT_BUFLEN+1];
	memset(num2,0,sizeof(num2));
	char recvbuf[DEFAULT_BUFLEN+1];
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
		scanf("%s",num1);//��ȡ����1 
		if (strlen(num1)==4&&num1[0]=='q'&&num1[1]=='u'&&num1[2]=='i'&&num1[3]=='t')//�ж��Ƿ����˳����� 
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
			//������������������Ϣ 
			unsigned int sendHead=htonl((unsigned int)strlen(num1));
			iResult=send(ConnectSocket,(char *)&sendHead,sizeof(unsigned int),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				break;
			}
			iResult=send(ConnectSocket,num1,(int)strlen(num1),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
			scanf("%s",num2);//��ȡ����2 
			sendHead=htonl((unsigned int)strlen(num2));//���������������2 
			iResult=send(ConnectSocket,(char *)&sendHead,sizeof(unsigned int),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				break;
			}
			iResult=send(ConnectSocket,num2,(int)strlen(num2),0);
			if (iResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}
		}
		//���շ��������ص���Ϣ 
		char *recvBuf=recvbuf;
		iResult=recvvl(ConnectSocket,recvBuf,DEFAULT_BUFLEN);
		if (iResult>0)
		{
			//�ɹ����������� 
			recvbuf[iResult]='\0';
			printf("%s\n",recvbuf);
		}
		else if (iResult==0)//���������ٷ����µ����� 
		{
			//�ر��׽��֣��ͷ���Դ 
			printf("Connection closed.\n");
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}
		else
		{
			if (iResult==-2)//���ݰ���С���������� 
			{
				printf("too big data!\n");//���������Ϣ 
				continue;//����ѭ�� 
			}
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			//��������ֱ�ӹر��׽��֣��˳�
			return 0;
		}
	} 

}
