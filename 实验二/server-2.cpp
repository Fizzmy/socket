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
const int P=27015;
#define MAX_PROCESS 2
char pt[10];
int totPro=0;
bool used[11];
HANDLE pro[11];
int createServer(char *recvbuf,sockaddr_in clientaddr,int clientlen)
{
	int id=0;
	for (int i=1;i<=MAX_PROCESS;i++)//Ѱ�ҿ��õĶ˿� 
		if (!used[i]) {id=i;break;}
		else
		{
			DWORD status=0;
			GetExitCodeProcess(pro[i],&status);
			if (status!=STILL_ACTIVE)//��ǰ�˿ڵ��ӳ����Ѿ����н��� 
			{
				printf("free PORT %d free.\n",i+P);
				used[i]=0;
				:: CloseHandle(pro[i]);
				id=i;
				break;
			}
		}
	if (id==0)//û�п��ö˿� 
	{
		printf("Too many process number!\n");
		return 0;
	}
	int port=id+P;
	int iResult;
	SOCKET ServerSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	ZeroMemory(&hints,sizeof(hints));
	//����UDPЭ����ز��� 
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_flags=AI_PASSIVE; 
	//������������ַ�Ͷ˿ں� 
	iResult=getaddrinfo("localhost",itoa(port,pt,10),&hints,&result);
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
	
	int iSendResult=sendto(ServerSocket,recvbuf,strlen(recvbuf),0,(SOCKADDR *)&clientaddr,clientlen); //���ͽ��յ������� 
	if (iSendResult==SOCKET_ERROR)
	{
		printf("send failed with error:%d\n",WSAGetLastError());
		closesocket(ServerSocket);
		return 0;
	}

	TCHAR szFilename[MAX_PATH] ;
    GetModuleFileName(NULL, szFilename, MAX_PATH) ;
	TCHAR socketId[MAX_PATH];
	sprintf(socketId,"\"%s\" %d",szFilename,(int)ServerSocket);
	// ָ���³����д��ڵ���� 
    STARTUPINFO si;
    ZeroMemory(reinterpret_cast <void*> (&si) , sizeof(si) ) ;//���������һ���ڴ����� 
    si.cb = sizeof(si) ;				// �����Ǳ��ṹ�Ĵ�С

    // ���ص������ӽ��̵Ľ�����Ϣ
    PROCESS_INFORMATION pi;

    // ����ͬ���Ŀ�ִ���ļ��������д������̣����������ӽ��̵�����
    BOOL bCreateOK=CreateProcess(
		szFilename,					// �������EXE��Ӧ�ó��������
		socketId,					// ��������Ϊ��һ���ӽ��̵ı�־
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
    	pro[id]=pi.hProcess;//��¼��� 
        :: CloseHandle(pi.hThread) ;
    }
    else return 0;
    return id;
}
void recvData(char *c)
{
	WSAData wsaData;
	SOCKET ServerSocket=atoi(c);
	char recvbuf[DEFAULT_BUFLEN+1]; 
	char echo[]="ECHO:";
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult,iSendResult;
	int recvbuflen=DEFAULT_BUFLEN;
	sockaddr_in clientaddr;
	int clientlen=sizeof(sockaddr_in);
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return;}
	printf("Connection established!\n");
	//���ó�ʱʱ�� 
	int nTimeOver=4000;
	setsockopt(ServerSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeOver,sizeof(nTimeOver));
	//�޸��׽ӿڵĹ���ģʽ ����Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ServerSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);
		if (iResult>=0)//���ܳɹ���������ս�� 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			iSendResult=sendto(ServerSocket,recvbuf,iResult,0,(SOCKADDR *)&clientaddr,clientlen); //���ͽ��յ������� 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ServerSocket);
				continue;
			}
		}
		else//���ճ��� 
		{
			if (WSAGetLastError()==10060)//��ʱ 
			{
				printf("Time out.Connection closed automatically.\n");
				break;
			}
			else printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ServerSocket);
			continue;
			//�ر��׽��֣��˳�ѭ�� 
		}
	}
	WSACleanup();
}
int main(int argc, char* argv[])
{
	if (argc>1)//�ӳ��� 
	{
		recvData(argv[1]);
		system("pause");
		return 0;
	}
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
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);
		if (iResult>=0)//���ܳɹ���������ս�� 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			char *recvBuf=recvbuf;
			int pt=createServer(recvBuf,clientaddr,clientlen);//�����ӳ��� 
			if (!pt)
			{
				printf("create process failed.\n");
				continue;
			}
			else
			{
				printf("used PORT %d.\n",pt+P);
				used[pt]=1;
			}
		}
		else//���ճ��� 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			continue;
		}
	}
	WSACleanup();
}
