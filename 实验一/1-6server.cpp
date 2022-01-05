#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstdio>
#include<iostream>
#include<time.h>
using namespace std;
#define DEFAULT_BUFLEN 10 //根据数字大小限制，缓冲区大小改为10（还要考虑符号） 
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
		iResult1=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);//接收第一个数字  
		if (iResult1==0)//客户端不再发送新的数据 
		{
			printf("Connection closed.\n");
			//通知客户端服务器不再发送新的数据
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
		else if (iResult1==-1)//接收出错 
		{
			printf("recv failed with error:%d\n",WSAGetLastError());
			closesocket(ClientSocket);
			break;
		}
		recvBuf=num2;
		iResult2=recvvl(ClientSocket,recvBuf,DEFAULT_BUFLEN);//接收第二个数字  
		if (iResult2==0)//和上面一样的处理 
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
		char error2[]="ERROR : integer out of range!";//报错信息 
		if (iResult1==10&&num1[0]!='-') iResult1=-2;//判断是否超出范围 
		if (iResult2==10&&num2[0]!='-') iResult2=-2;
		if (iResult1>0&&iResult2>0)
		{
			num1[iResult1]='\0';
			num2[iResult2]='\0';
			printf("%s %s\n",num1,num2);
			bool can=1;//判断是否是整数 
			int a=0,b=0;
			int zf1=1,zf2=1;
			if (num1[0]=='-')
			{
				zf1=-1;//负数 
				if (iResult1==1) can=0;//只有一个负号 
				for (int i=1;i<iResult1;i++)
					if (num1[i]>='0'&&num1[i]<='9') a=a*10+num1[i]-'0';//字符转成数字 
					else {can=0;break;} //出现非数字字符 
			}
			else
			{//正数 
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
			if (!can)//不是整数 
			{
				unsigned int sendHead=htonl(strlen(error1));//发给客户端报错1 
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
			{//都是整数 
				int c=a+b;//相加后转换成字符串 
				itoa(c,num3,10);
				unsigned int sendHead=htonl(strlen(num3));//发给客户端结果 
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
			//整数超出范围 
			unsigned int sendHead=htonl(strlen(error2));//发给客户端报错2
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

//	cout<<s<<endl;
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
		if (!createServer(ClientSocket))
		{
			printf("create process failed\n");
			continue;
		} 
	}
}
