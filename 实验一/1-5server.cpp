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
void recvData(char *c)//c�д���������׽��־�� 
{
	WSAData wsaData;
	SOCKET ClientSocket=atoi(c);//�ַ���ת�������� 
	char recvbuf[DEFAULT_BUFLEN+1]; 
	char echo[]="ECHO:";
	char error[]="ERROR:too big data!\n";
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult,iSendResult;
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return;}
	printf("Connection established!\n");
	while (1)//����ĳ�����ϸ�ʵ��䳤����һ�� 
	{
		char *recvBuf=recvbuf;
		iResult=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);
		if (iResult>0)
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			iResult+=strlen(echo);
			unsigned int sendHead=htonl(iResult);
			iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
			iSendResult=send(ClientSocket,echo,strlen(echo),0); 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
			iSendResult=send(ClientSocket,recvbuf,iResult-strlen(echo),0); 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
		}
		else if (iResult==0)
		{
			printf("Connection closed.\n");
			//�ر����� 
			iResult=shutdown(ClientSocket,SD_SEND);
			if (iResult==SOCKET_ERROR)
			{
				printf("shutdown failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
			//�ر��׽���
			closesocket(ClientSocket);
			break;
		}
		else
		{
			if (iResult==-2)
			{
				printf("too big data!\n");
				unsigned int sendHead=htonl(strlen(error));
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
				continue;
			}
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ClientSocket);
			break;
		}
	}
}
bool createServer(SOCKET s)
{
	// ��ȡ��ǰ�ļ��ľ���·��
	TCHAR szFilename[MAX_PATH] ;
    GetModuleFileName(NULL, szFilename, MAX_PATH) ;
	TCHAR socketId[MAX_PATH];
	sprintf(socketId,"\"%s\" %d",szFilename,(int)s);
	// ָ���³����д��ڵ���� 
    STARTUPINFO si;
    ZeroMemory(reinterpret_cast <void*> (&si) , sizeof(si) ) ;//���������һ���ڴ����� 
    si.cb = sizeof(si) ;				// �����Ǳ��ṹ�Ĵ�С

    // ���ص������ӽ��̵Ľ�����Ϣ
    PROCESS_INFORMATION pi;

    // ����ͬ���Ŀ�ִ���ļ��������д������̣����������ӽ��̵�����
    BOOL bCreateOK=CreateProcess(
		szFilename,					// �������EXE��Ӧ�ó��������
		socketId,					// ��������Ϊ��һ���ӽ��̵ı�־�����ӽ��̴��� 
		NULL,						// ȱʡ�Ľ��̰�ȫ��
		NULL,						// ȱʡ���̰߳�ȫ��
		TRUE,						// �̳о��
		CREATE_NEW_CONSOLE,			// ʹ���µĿ���̨
		NULL,						// �µĻ���
		NULL,						// ��ǰĿ¼
		&si,						// ������Ϣ
		&pi) ;						// ���صĽ�����Ϣ
	if (bCreateOK)
    {
        :: CloseHandle(pi.hProcess) ;
        :: CloseHandle(pi.hThread) ;
    }
    else return 0;
    return 1;
}
int main(int argc, char* argv[])
{
	
	if (argc>1)
	{
		recvData(argv[1]);
		system("pause");
		return 0;
	}
	WSADATA wsaData;
	SOCKET ListenSocket=INVALID_SOCKET;
	SOCKET ClientSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,*ptr=NULL,hints;
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
		if (iResult==SOCKET_ERROR) 
		{
			printf("accept failed with error:%d\n",WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}
		printf("Connection established!\n");
		if (!createServer(ClientSocket))//�����ӽ��̣��������׽��־�����ݸ��ӽ��� 
		{
			printf("create process failed\n");
			continue;
		} 
	}
}
