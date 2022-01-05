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
void recvData(char *c)//c中存的是连接套接字句柄 
{
	WSAData wsaData;
	SOCKET ClientSocket=atoi(c);//字符串转化成整型 
	char recvbuf[DEFAULT_BUFLEN+1]; 
	char echo[]="ECHO:";
	char error[]="ERROR:too big data!\n";
	memset(recvbuf,0,sizeof(recvbuf));
	int iResult,iSendResult;
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return;}
	printf("Connection established!\n");
	while (1)//这里的程序和上个实验变长接收一样 
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
			//关闭连接 
			iResult=shutdown(ClientSocket,SD_SEND);
			if (iResult==SOCKET_ERROR)
			{
				printf("shutdown failed with error:%d\n",WSAGetLastError());
				closesocket(ClientSocket);
				break;
			}
			//关闭套接字
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
	// 获取当前文件的绝对路径
	TCHAR szFilename[MAX_PATH] ;
    GetModuleFileName(NULL, szFilename, MAX_PATH) ;
	TCHAR socketId[MAX_PATH];
	sprintf(socketId,"\"%s\" %d",szFilename,(int)s);
	// 指明新程序中窗口的外观 
    STARTUPINFO si;
    ZeroMemory(reinterpret_cast <void*> (&si) , sizeof(si) ) ;//用零来填充一块内存区域 
    si.cb = sizeof(si) ;				// 必须是本结构的大小

    // 返回的用于子进程的进程信息
    PROCESS_INFORMATION pi;

    // 利用同样的可执行文件和命令行创建进程，并赋于其子进程的性质
    BOOL bCreateOK=CreateProcess(
		szFilename,					// 产生这个EXE的应用程序的名称
		socketId,					// 告诉其行为像一个子进程的标志，向子进程传参 
		NULL,						// 缺省的进程安全性
		NULL,						// 缺省的线程安全性
		TRUE,						// 继承句柄
		CREATE_NEW_CONSOLE,			// 使用新的控制台
		NULL,						// 新的环境
		NULL,						// 当前目录
		&si,						// 启动信息
		&pi) ;						// 返回的进程信息
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
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置TCP协议相关参数 
	hints.ai_family=AF_UNSPEC;//根据ipv4和ipv6做不同选择
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	hints.ai_flags=AI_PASSIVE; 
	//解析服务器地址和端口号 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//创建监听套接字 
	ListenSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ListenSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//将监听套接字绑定地址和端口号 
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
	//监听连接请求 
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
		//接受连接请求，返回连接套接字 
		ClientSocket=accept(ListenSocket,NULL,NULL);
		if (iResult==SOCKET_ERROR) 
		{
			printf("accept failed with error:%d\n",WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}
		printf("Connection established!\n");
		if (!createServer(ClientSocket))//创建子进程，把连接套接字句柄传递给子进程 
		{
			printf("create process failed\n");
			continue;
		} 
	}
}
