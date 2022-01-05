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
	for (int i=1;i<=MAX_PROCESS;i++)//寻找可用的端口 
		if (!used[i]) {id=i;break;}
		else
		{
			DWORD status=0;
			GetExitCodeProcess(pro[i],&status);
			if (status!=STILL_ACTIVE)//当前端口的子程序已经运行结束 
			{
				printf("free PORT %d free.\n",i+P);
				used[i]=0;
				:: CloseHandle(pro[i]);
				id=i;
				break;
			}
		}
	if (id==0)//没有可用端口 
	{
		printf("Too many process number!\n");
		return 0;
	}
	int port=id+P;
	int iResult;
	SOCKET ServerSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	ZeroMemory(&hints,sizeof(hints));
	//配置UDP协议相关参数 
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_flags=AI_PASSIVE; 
	//解析服务器地址和端口号 
	iResult=getaddrinfo("localhost",itoa(port,pt,10),&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//创建UDP套接字 
	ServerSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ServerSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//将UDP套接字绑定地址和端口号 
	iResult=bind(ServerSocket,result->ai_addr,(int)result->ai_addrlen);
	
	int iSendResult=sendto(ServerSocket,recvbuf,strlen(recvbuf),0,(SOCKADDR *)&clientaddr,clientlen); //发送接收到的数据 
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
	// 指明新程序中窗口的外观 
    STARTUPINFO si;
    ZeroMemory(reinterpret_cast <void*> (&si) , sizeof(si) ) ;//用零来填充一块内存区域 
    si.cb = sizeof(si) ;				// 必须是本结构的大小

    // 返回的用于子进程的进程信息
    PROCESS_INFORMATION pi;

    // 利用同样的可执行文件和命令行创建进程，并赋于其子进程的性质
    BOOL bCreateOK=CreateProcess(
		szFilename,					// 产生这个EXE的应用程序的名称
		socketId,					// 告诉其行为像一个子进程的标志
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
    	pro[id]=pi.hProcess;//记录句柄 
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
	//设置超时时间 
	int nTimeOver=4000;
	setsockopt(ServerSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeOver,sizeof(nTimeOver));
	//修改套接口的工作模式 避免Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ServerSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);
		if (iResult>=0)//接受成功，输出接收结果 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			iSendResult=sendto(ServerSocket,recvbuf,iResult,0,(SOCKADDR *)&clientaddr,clientlen); //发送接收到的数据 
			if (iSendResult==SOCKET_ERROR)
			{
				printf("send failed with error:%d\n",WSAGetLastError());
				closesocket(ServerSocket);
				continue;
			}
		}
		else//接收出错 
		{
			if (WSAGetLastError()==10060)//超时 
			{
				printf("Time out.Connection closed automatically.\n");
				break;
			}
			else printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ServerSocket);
			continue;
			//关闭套接字，退出循环 
		}
	}
	WSACleanup();
}
int main(int argc, char* argv[])
{
	if (argc>1)//子程序 
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
	//初始化套接字 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//配置UDP协议相关参数 
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_flags=AI_PASSIVE; 
	//解析服务器地址和端口号 
	iResult=getaddrinfo("localhost",DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	//创建UDP套接字 
	ServerSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ServerSocket==INVALID_SOCKET)
    {
		printf("socket failed with error:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}
	//将UDP套接字绑定地址和端口号 
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
	//修改套接口的工作模式 避免Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ServerSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	while (1)
	{
		ZeroMemory(&clientaddr,sizeof(clientaddr));
		iResult=recvfrom(ServerSocket,recvbuf,recvbuflen,0,(SOCKADDR *)&clientaddr,&clientlen);
		if (iResult>=0)//接受成功，输出接收结果 
		{
			recvbuf[iResult]='\0';
			fputs(recvbuf,stdout);
			
			int len=strlen(echo);
			for (int i=iResult;i>=0;i--) recvbuf[i+len]=recvbuf[i];
			for (int i=0;i<len;i++) recvbuf[i]=echo[i];
			iResult+=len;
			char *recvBuf=recvbuf;
			int pt=createServer(recvBuf,clientaddr,clientlen);//创建子程序 
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
		else//接收出错 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			continue;
		}
	}
	WSACleanup();
}
