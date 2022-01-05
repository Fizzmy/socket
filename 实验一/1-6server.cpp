#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstdio>
#include<iostream>
#include<time.h>
using namespace std;
#define DEFAULT_BUFLEN 10 //�������ִ�С���ƣ���������С��Ϊ10����Ҫ���Ƿ��ţ� 
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
void recvData(char *c)
{
	WSAData wsaData;
	SOCKET ClientSocket=atoi(c);
	char num1[DEFAULT_BUFLEN+1],num2[DEFAULT_BUFLEN+1],num3[DEFAULT_BUFLEN+2]; 
	char echo[]="ECHO:";
	memset(num1,0,sizeof(num1));
	memset(num2,0,sizeof(num2));
	int iResult,iResult1,iResult2,iSendResult;
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return;}
	printf("Connection established!\n");
	while (1)
	{
		char *recvBuf=num1;
		iResult1=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);//���յ�һ������  
		if (iResult1==0)//�ͻ��˲��ٷ����µ����� 
		{
			printf("Connection closed.\n");
			//֪ͨ�ͻ��˷��������ٷ����µ�����
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
		else if (iResult1==-1)//���ճ��� 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ClientSocket);
			break;
		}
		recvBuf=num2;
		iResult2=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);//���յڶ�������  
		if (iResult2==0)//������һ���Ĵ��� 
		{
			printf("Connection closed.\n");
			iResult=shutdown(ClientSocket,SD_SEND);
			if (iResult==SOCKET_ERROR)
			{
				printf("shutdown failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
			closesocket(ClientSocket);
			break;
		}
		else if (iResult2==-1)
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ClientSocket);
			break;
		}
		char error1[]="ERROR : not a integer!";
		char error2[]="ERROR : integer out of range!";//������Ϣ 
		if (iResult1==10&&num1[0]!='-') iResult1=-2;//�ж��Ƿ񳬳���Χ 
		if (iResult2==10&&num2[0]!='-') iResult2=-2;
		if (iResult1>0&&iResult2>0)
		{
			num1[iResult1]='\0';
			num2[iResult2]='\0';
			printf("%s %s\n",num1,num2);
			bool can=1;//�ж��Ƿ������� 
			int a=0,b=0;
			int zf1=1,zf2=1;
			if (num1[0]=='-')
			{
				zf1=-1;//���� 
				if (iResult1==1) can=0;//ֻ��һ������ 
				for (int i=1;i<iResult1;i++)
					if (num1[i]>='0'&&num1[i]<='9') a=a*10+num1[i]-'0';//�ַ�ת������ 
					else {can=0;break;} //���ַ������ַ� 
			}
			else
			{//���� 
				for (int i=0;i<iResult1;i++)
					if (num1[i]>='0'&&num1[i]<='9') a=a*10+num1[i]-'0';
					else {can=0;break;}
			}
			a*=zf1;
			if (num2[0]=='-')
			{
				zf2=-1;
				if (iResult2==1) can=0;
				for (int i=1;i<iResult2;i++)
					if (num2[i]>='0'&&num2[i]<='9') b=b*10+num2[i]-'0';
					else {can=0;break;} 
			}
			else
			{
				for (int i=0;i<iResult2;i++)
					if (num2[i]>='0'&&num2[i]<='9') b=b*10+num2[i]-'0';
					else {can=0;break;}
			}
			b*=zf2;
			if (!can)//�������� 
			{
				unsigned int sendHead=htonl(strlen(error1));//�����ͻ��˱���1 
				iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					return;
				}
				iSendResult=send(ClientSocket,error1,strlen(error1),0); 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					return;
				}
			}
			else
			{//�������� 
				int c=a+b;//��Ӻ�ת�����ַ��� 
				itoa(c,num3,10);
				unsigned int sendHead=htonl(strlen(num3));//�����ͻ��˽�� 
				iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					return;
				}
				iSendResult=send(ClientSocket,num3,strlen(num3),0); 
				if (iSendResult==SOCKET_ERROR)
				{
					printf("send failed with error:%d\n",WSAGetLastError());
					closesocket(ClientSocket);
					return;
				}
			}
		}
		else
		{
			//����������Χ 
			unsigned int sendHead=htonl(strlen(error2));//�����ͻ��˱���2
			iSendResult=send(ClientSocket,(char *)&sendHead,sizeof(unsigned int),0);
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				return;
			}
			iSendResult=send(ClientSocket,error2,strlen(error2),0); 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				return;
			}
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

//	cout<<s<<endl;
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
	//cout<<bCreateOK<<endl;
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
		if (!createServer(ClientSocket))
		{
			printf("create process failed\n");
			continue;
		} 
	}
}
