#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<ctime>
#include<iostream>
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
using namespace std;
struct rtt_info {
	double rtt_rtt;	/* most recent measured RTT, seconds */
	double rtt_srtt;	/* smoothed RTT estimator, seconds */
	double rtt_rttvar;	/* smoothed mean deviation, seconds */
	double rtt_rto;	/* current RTO to use, seconds */
	int rtt_nrexmt;	/* #times retransmitted: 0, 1, 2, ... */
	time_t rtt_base;	/* #sec since 1/1/1970 at start */
}rtt;
#define	RTT_RXTMIN      2	/* RTT����Сֵ */
#define	RTT_RXTMAX     60	/* RTT���ֵ */
#define	RTT_MAXNREXMT 	3	/* ���²���RTT�������� */
#define	RTT_RTOCALC(ptr) ((ptr)->rtt_srtt + (4.0 * (ptr)->rtt_rttvar)) //����RTT 
double rtt_minmax(double rto)//��ֹRTTԽ�� 
{
	if (rto < RTT_RXTMIN)
		rto = RTT_RXTMIN;
	else if (rto > RTT_RXTMAX)
		rto = RTT_RXTMAX;
	return rto;
}
void init(rtt_info *ptr)//RTT���ݽṹ��ʼ�� 
{
	ptr->rtt_base = time(0);//ʱ���
	ptr->rtt_rtt    = 0;
	ptr->rtt_srtt   = 0;
	ptr->rtt_rttvar = 0.75;
	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}
int setTimeout(SOCKET ConnectLessSocket,int nTimeOver)//���ó�ʱʱ�� 
{
	return setsockopt(ConnectLessSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeOver,sizeof(nTimeOver));
}
struct header{
	int seq;
	int time;
}sendmsg,recvmsg;
bool rttinit=0;
int rtt_timeout(struct rtt_info *ptr)
{
	ptr->rtt_rto *= 2;		/* next RTO */
	if (++ptr->rtt_nrexmt > RTT_MAXNREXMT)
		return -1;			/* time to give up for this packet */
	return 0;
} 
void rtt_stop(struct rtt_info *ptr, time_t ms)
{
	double		delta;
	ptr->rtt_rtt = ms;		/* measured RTT in seconds */
	/*
	 * Update our estimators of RTT and mean deviation of RTT.
	 * See Jacobson's SIGCOMM '88 paper, Appendix A, for the details.
	 * We use floating point here for simplicity.
	 */

	delta = ptr->rtt_rtt - ptr->rtt_srtt;
	ptr->rtt_srtt += delta / 8;		/* g = 1/8 */

	if (delta < 0.0)
		delta = -delta;				/* |delta| */

	ptr->rtt_rttvar += (delta - ptr->rtt_rttvar) / 4;	/* h = 1/4 */

	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}
int main()
{
	WSADATA wsaData;
	SOCKET ConnectLessSocket=INVALID_SOCKET;
	struct addrinfo *result=NULL,hints;
	int iResult;
	int recvbuflen=DEFAULT_BUFLEN;
	char sendbuf[DEFAULT_BUFLEN];
	memset(sendbuf,0,sizeof(sendbuf));
	char recvbuf[DEFAULT_BUFLEN+sizeof(header)];
	memset(recvbuf,0,sizeof(recvbuf));
	//��ʼ���׽��� 
	iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
	if (iResult!=0) {printf("WSAStartup failed with error:%d\n",iResult);return 0;}
	ZeroMemory(&hints,sizeof(hints));
	//����UDPЭ����ز���
	hints.ai_family=AF_INET;//ipv4
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	//������������ַ�Ͷ˿ں� 
	char *ip="localhost";
	iResult=getaddrinfo(ip,DEFAULT_PORT,&hints,&result);
	if (iResult!=0){printf("getaddrinfo failed with error:%d\n",iResult);WSACleanup();return 0;}
	ConnectLessSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if (ConnectLessSocket==INVALID_SOCKET)
    {
		printf("socket failed with error: %ld\n",WSAGetLastError());
		WSACleanup();
		return 0;
	}
	//�޸��׽ӿڵĹ���ģʽ ����Winsock bug 
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ConnectLessSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	
	while (1) 
	{
		if (rttinit==0)//û�г�ʼ��RTT 
		{
			init(&rtt);//��ʼ��RTT
			rttinit=1;
		}
		fgets(sendbuf,DEFAULT_BUFLEN,stdin);//��ȡһ������ 
		if (strlen(sendbuf)==5&&sendbuf[0]=='q'&&sendbuf[1]=='u'&&sendbuf[2]=='i'&&sendbuf[3]=='t'&&sendbuf[4]=='\n')//�ж��ǲ���quit 
		{
			closesocket(ConnectLessSocket);
			WSACleanup();
			return 0;
		}
		recvmsg.seq=0;
		sendmsg.seq++;//����ͷ��� 
		rtt.rtt_nrexmt=0;//���ó�ʱ���� 
		while (1)
		{
			bool success=0;
			printf("%.3f\n",rtt.rtt_rto);//�����ǰ��RTT 
			sendmsg.time=time(0)-rtt.rtt_base;//�޸ķ���ͷ 
			char tmpBuf[sizeof(header)+DEFAULT_BUFLEN];
			
			header *p = (header *)tmpBuf;//����ͷ����Ϣ��װ 
			p->seq=sendmsg.seq;
			p->time=sendmsg.time;
			memcpy((char*)p+sizeof(int)+sizeof(time_t),sendbuf,strlen(sendbuf));	
			
			//���������ͷ�װ��ĵ���Ϣ 
			iResult=sendto(ConnectLessSocket,tmpBuf,sizeof(int)+sizeof(time_t)+strlen(sendbuf),0,result->ai_addr,(int)result->ai_addrlen);
			if (iResult==SOCKET_ERROR)
			{
				printf("sendto failed with error:%d\n",WSAGetLastError());
				closesocket(ConnectLessSocket);
				WSACleanup();
				return 0;
			}
			
			time_t base=time(0);//�趨��ʼ����ǰ��ʱ�� 
			while (1)
			{
				//cout<<(int)(rtt.rtt_rto*1000+0.5)-(time(0)-base)<<endl;
				if (setTimeout(ConnectLessSocket,(int)(rtt.rtt_rto*1000+0.5)-(time(0)-base)))
				//���ý��ճ�ʱʱ�䣬��ʱʱ��ΪRTTʱ��-(��ǰʱ��-��ʼ����ǰ��ʱ��) 
					printf("set time out error: %d\n",WSAGetLastError());
				
				iResult=recvfrom(ConnectLessSocket,recvbuf,recvbuflen,0,NULL,NULL);//�ӷ��������շ��ص���Ϣ 
				if (iResult>=0)//�ɹ������� 
				{
					printf("recv %d btyes.\n",iResult);
					header *p = (header *)recvbuf;
			//		for (int i=0;i<iResult;i++)
			//			printf("%d ",recvbuf[i]);printf("\n");
			//		cout<<sendmsg.seq<<" "<<p->seq<<endl;
			//		cout<<sendmsg.time<<" "<<p->time<<endl;
					if (p->seq==sendmsg.seq&&p->time==sendmsg.time)//����Ǹղŷ��͵İ���ô��ǳɹ������� 
					{
						rtt_stop(&rtt,time(0)-rtt.rtt_base-p->time);//�޸�RTT 
						success=1;
						break;
					}
				}
				else
				{
					if (WSAGetLastError()==10060)//��ʱ 
					{
						if (rtt_timeout(&rtt)<0)//�ı�RTT��������ʱ�������޾ͷ����ð�������RTT 
						{
							printf("no response from server, giving up\n");
							rttinit=0;
							break;
						}
						printf("Time out.Resending...\n");//û�������ޣ����·��͸ð� 
						break;
						
					}
					else printf("recv failed with error:%d\n",WSAGetLastError());//�������� 
					closesocket(ConnectLessSocket);
					WSACleanup();
					return 0;
				}
			}
			if (success==1||rttinit==0) break;
		}
	}
}
