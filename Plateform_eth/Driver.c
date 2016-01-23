
#include <stdio.h>

#include "../Plateform_header/Driver.h"
#include "../Plateform_header/Protocol.h"

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <errno.h>

#include <pthread.h>
#include <dlfcn.h>
#endif
//int fd;
ISuperProtocol pProtocol;
IProtocol *currplc;
void Driver_RxThread (int status);

U32 u32NodeObjIdx=0;


U32 Driver_Send(U32 fd, U8 *pBuf, U32 nByte)
{
#ifdef WIN32
	FD_SET rfds = {1, fd};
	TIMEVAL tv = {2, 0};
	U8 ppch[255];
	U8 bReceived=0;
	volatile U8 *pRxStatus=0;
	volatile U8 *pU8Cnt;
	int i;
	
	send(fd, (char*)pBuf, nByte, 0);
	pRxStatus = GetRxStatus();
	pU8Cnt = Protocol_GetU8CntPtr();
	*pRxStatus=0;
	*pU8Cnt=0;
	while((*pRxStatus)==0)
	{
		select(0, &rfds, NULL, NULL, &tv);
		if((bReceived=recv(fd, (char*)ppch, 11, 0))>1 && bReceived!=0xff)
		{
			for(i=0;i<bReceived;i++)
			{
				Protocol_CommuISR(currplc, ppch[i]);
			}
		}
	}
#else
	int i=0, bReceived=0, j=0;
	U8 u8rbuffer[128];
	volatile U8 *pRxStatus;
	volatile U8 *pU8Cnt;

#ifdef _SELECT_	
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec=2;
	tv.tv_usec=0;
#endif
	
	DegPrint("send(%d):",fd);
	for(i=0; i<nByte; i++)
	{
		DegPrint("%x, ",pBuf[i]);
	}DegPrint("\n");

	if(send(fd, pBuf, nByte, 0)== -1)
	{
		switch(errno)
		{
			case EINTR:
				for(i=0; send(fd, pBuf, nByte, 0)< 0; i++)
				{
					if(i == 3)
					{//consecutive error 3 times
						return 2;	//return reset connection notification
					}
				}
				break;
				
			default:
				return 2;	//return reset connection notification
		}
	}
	pRxStatus = GetRxStatus();
	pU8Cnt = Protocol_GetU8CntPtr();
	*pRxStatus=0;
	*pU8Cnt=0;
#ifndef _SELECT_	
	while((*pRxStatus)==0)
	{//printf("b");struct timeval tv;gettimeofday(&tv, NULL);printf("%d, %d\n", tv.tv_sec,tv.tv_usec);
		bReceived = recv(fd, u8rbuffer, sizeof(u8rbuffer), MSG_DONTWAIT);//printf("a");gettimeofday(&tv, NULL);printf("%d, %d\n", tv.tv_sec,tv.tv_usec);fflush(stdout);
			if(bReceived > 1 && bReceived!=0xff)
			{
				for(i=0;i<bReceived;i++)
				{
					Protocol_CommuISR(currplc, u8rbuffer[i]);
				}
			}
			j++;
			if(j>20)
			{//timeout
				printf("timeout\n");
				return 2;
			}
		usleep(50000);
	}
	
#else	//select version has not been tested stablely
	while((*pRxStatus)==0)
	{printf("b\n");
		retval = select(fd+1, &rfds, NULL, NULL, &tv);printf("a\n");
		if(retval == -1)
		{
			printf("err\n");
			return 2;
		}
		else if(retval == 0)
		{
			printf("select timeout:%d\n",fd);
			return 2;
		}
		else if(FD_ISSET(fd, &rfds))
		{
			bReceived = recv(fd, u8rbuffer, sizeof(u8rbuffer), MSG_DONTWAIT);//printf("Rec:%x %d\n",bReceived,j);
			if(bReceived > 1 && bReceived!=0xff)
			{
				for(i=0;i<bReceived;i++)
				{
					Protocol_CommuISR(currplc, u8rbuffer[i]);
				}
			}
			j++;
			if(j>1000)
			{//timeout
				printf("timeout\n");
				return 2;
			}
		}
		else
		{//timeout
			printf("timeout2\n");
			return 2;
		}
		//usleep(500000);
	}
#endif
#endif//end win32	
	return 1;//success
}

#ifdef _Multithread_
#ifdef WIN32
CRITICAL_SECTION CriticalSection;
#else
#include<semaphore.h>
sem_t sem_sending;
#endif
#endif

U8 u8InitState=0;

int Driver_Connect(CommuPara *pPara, int socketfd)
{
	struct sockaddr_in dest;
	int iConnectTime,a, last_error;

#ifdef WIN32
	dest.sin_family=AF_INET;
	dest.sin_port=htons(pPara->CommuType.ethernet.u16Port);
	dest.sin_addr.s_addr=inet_addr(pPara->CommuType.ethernet.strIP);//todo
#else
	bzero(&dest, sizeof(dest));
	dest.sin_family=PF_INET;
	dest.sin_port=htons(pPara->CommuType.ethernet.u16Port);
	inet_aton(pPara->CommuType.ethernet.strIP, &dest.sin_addr);
#endif	
	DegPrint("ip:%s\n",pPara->CommuType.ethernet.strIP);

	//printf("123\n");struct timeval tv;gettimeofday(&tv, NULL);printf("%d, %d\n", tv.tv_sec,tv.tv_usec);
	for(iConnectTime=0; (a=connect(socketfd, (struct sockaddr*)&dest, sizeof(dest)))!=0; iConnectTime++)
	{//printf("125\n");
		printf("fail fd:%d %d\n",socketfd,errno);
#ifdef WIN32		
		last_error = WSAGetLastError();
		if(last_error==10035)
			return 1;
#endif			
		if(iConnectTime == 3)
		{
#ifdef WIN32
			closesocket((SOCKET)socketfd);
#else
			close(socketfd);
#endif
			return 0;//fail
		}
		#ifdef WIN32			
			Sleep(10000);//休10secs後再重建連線
		#else
			sleep(10);
		#endif
	}printf("132:%d %d\n",a,socketfd);//gettimeofday(&tv, NULL);printf("%d, %d\n", tv.tv_sec,tv.tv_usec);fflush(stdout);
	
	return 1;
}

U32 Driver_InitDriver(CommuPara *pPara)
{
#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 0);
	SOCKET sSocket=0;
	int fd;
	int nErrCode;
	ULONG cmd_option=1;
	WSAStartup(wVersionRequested,&wsaData);
	
	if((sSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		nErrCode=WSAGetLastError();
		sSocket=0;
		return FALSE;
	}
   //change socket to non-blocking mode
	if (ioctlsocket(sSocket, FIONBIO, &cmd_option) == SOCKET_ERROR)
   		return FALSE;

	fd = (int)sSocket;//SOCKET is an integer type in windows plateform
#else
	struct sockaddr_in dest;
	char buffer[128];
	int flag;
	int fd=0;
	

	fd = socket(PF_INET, SOCK_STREAM, 0);
	
	//set to non-blocking mode
	flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag, O_NONBLOCK);
#endif	
	if(Driver_Connect(pPara, fd)==0)
		return 0;
	
#ifdef _Multithread_	
	if(u8InitState==0)
	{
#ifdef WIN32
		InitializeCriticalSection(&CriticalSection);

#else
		sem_init(&sem_sending, 0, 1);
		u8InitState=1;
#endif
	}
#endif		
	
	return fd;
  
}


void Driver_LockRes()
{
#ifdef _Multithread_
#ifdef WIN32
	EnterCriticalSection(&CriticalSection);
#else
	sem_wait(&sem_sending);
#endif
#endif
}

void Driver_UnlockRes()
{
#ifdef _Multithread_	
#ifdef WIN32
	LeaveCriticalSection(&CriticalSection);
#else
	sem_post(&sem_sending);
#endif
#endif	
}

//u8Len in byte
//*u8State 1: success  2:err
//void Driver_ReadPLC(U32 fd, char *label, U8 u8StnID, U8 u8Len, U8 *pbuf, U8 *u8State)
void Driver_ReadPLC_Core(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, U8 u8Sync)
{
	U8 buf[255];//放cmnd轉出plc command的結果
	U8 u8Txlen = 0;
	PLCIOCmnd cmnd;
	U8 u8ProtocolState = 0;
	IProtocol *plc;
	
	if(*(pnode->handle) == 0)
	{
		*(pnode->handle) = Driver_InitDriver(pnode->para);
		if(*(pnode->handle) == 0)
		{
			#ifdef WIN32			
				Sleep(120000);//休120secs後再重建連線
			#else
				sleep(120);
			#endif
			*u8State=2;
			return;
		}
	}
	
	U8 u8StnID = pnode->u8Stnid;
	
	cmnd.pBuf = pbuf;
	cmnd.pState = &u8ProtocolState;
	cmnd.fd=*(pnode->handle);
	
	if(u8Sync == 1)
		Driver_LockRes();

	plc = (IProtocol*)pnode->pplc;//Driver_GetInterfaceByNode(u8StnID);
	currplc = plc;//printf("currplc:%x\n",(int)currplc);
	plc->Label2Cmnd(label, u8StnID, u8Len, &cmnd);
	plc->GenReadTelegram(&cmnd, buf, &u8Txlen);

	switch(Driver_Send(*(pnode->handle), buf,u8Txlen))
	{
		case 2:
			//reset connection and re-establish the socket
			if(u8Sync == 1)
				Driver_UnlockRes();
#ifdef WIN32
			closesocket((SOCKET)pnode->handle);
#else
			close(*(pnode->handle));printf("close1\n");
#endif
#ifdef WIN32			
			Sleep(120000);//休120secs後再重建連線
#else
			sleep(120);
#endif
			*(pnode->handle) = Driver_InitDriver(pnode->para);printf("346:%d\n",*(pnode->handle));

			*u8State=2;
			return;
		
		case 1://ok

			break;	
	}

	if(Protocol_WaitPLCResp())
	{//ok
		plc->ParseReadTelegram(&cmnd);
		
		switch(u8ProtocolState)
		{
			case 1://success
				*u8State=1;
				break;
		  default:
		  	*u8State=0x80|u8ProtocolState;//something wrong in the driver >0x80定義為protocol層錯誤
		  	break;
		}
		if(u8Sync == 1)
			Driver_UnlockRes();
		
		return;
	}
	*u8State=2;
	if(u8Sync == 1)
		Driver_UnlockRes();

	//not ok
	
}

void Driver_WritePLC_Core(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, U8 u8Sync)
{
	U8 buf[255];//放cmnd轉出plc command的結果
	U8 u8Txlen = 0;
	PLCIOCmnd cmnd;
	IProtocol *plc;
	U8 u8StnID;
	U8 u8ProtocolState = 0;
	
	if(*(pnode->handle) == 0)
	{
		*(pnode->handle) = Driver_InitDriver(pnode->para);
		if(*(pnode->handle) == 0)
		{
			#ifdef WIN32			
				Sleep(120000);//休120secs後再重建連線
			#else
				sleep(120);
			#endif
			*u8State=2;
			return;
		}
	}
	
	
	cmnd.pBuf = pbuf;
	cmnd.pState = &u8ProtocolState;
	cmnd.fd=*(pnode->handle);
	
	u8StnID = pnode->u8Stnid;
	
	if(u8Sync == 1)
		Driver_LockRes();

	plc = (IProtocol*)pnode->pplc;
	currplc = plc;//printf("currplc:%x\n",(int)currplc);
	plc->Label2Cmnd(label, u8StnID, u8Len, &cmnd);
	plc->GenWriteTelegram(&cmnd, buf, &u8Txlen);

	switch(Driver_Send(*(pnode->handle), buf,u8Txlen))
	{
		case 2:
			//reset connection and re-establish the socket
			if(u8Sync == 1)
				Driver_UnlockRes();
#ifdef WIN32
			closesocket((SOCKET)pnode->handle);
#else
			close(*(pnode->handle));
#endif
			
#ifdef WIN32			
			Sleep(120000);//休120secs後再重建連線
#else
			sleep(120);
#endif
			*(pnode->handle) = Driver_InitDriver(pnode->para);

				*u8State=2;
				return;
		
		case 1://ok
			break;	
	}
	
	if(Protocol_WaitPLCResp())
	{//ok
		plc->ParseWriteTelegram(&cmnd);
		switch(u8ProtocolState)
		{
			case 1://success
				*u8State=1;
				break;
		  case 2:
		  	*u8State=0x80;//something wrong in the driver >0x80定義為protocol層錯誤
		  	break;
		}
		if(u8Sync == 1)
			Driver_UnlockRes();
		return;
	}
	*u8State=2;
	if(u8Sync == 1)
		Driver_UnlockRes();
	//not ok
	
}

void Driver_ReadPLC(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State)
{
	Driver_ReadPLC_Core(pnode, label, u8Len, pbuf, u8State, 1);
}
void Driver_WritePLC(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State)
{
	Driver_WritePLC_Core(pnode, label, u8Len, pbuf, u8State, 1);
}

void Driver_WritePLC2(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, char *mask)
{//assume 16bit system mask[0] is the lowest bit
	U8 *pu8Buf;
	U16 *pu16WBuf;
	int i;
	pu8Buf= (U8*)malloc(u8Len);memset(pu8Buf, 0, u8Len);
	pu16WBuf = (U16*)malloc(u8Len);memset(pu16WBuf, 0, u8Len);
	
	Driver_LockRes();
	Driver_ReadPLC_Core(pnode, label, u8Len, pu8Buf, u8State,0);
	if(*u8State != 1)
	{
		Driver_UnlockRes();
		return;
	}
	
	for(i=0; i<16*u8Len; i++)
	{
		if(mask[i] == 'f')
		{
			(*pu16WBuf) += (*(U16*)pu8Buf) & (1<<i);
		}
		else if(mask[i] == '1')
		{
			(*pu16WBuf) += (1<<i);
		}
		else
		{
		}
		
	}
	*(U16*)pbuf = *(U16*)pu16WBuf;
	Driver_WritePLC_Core(pnode, label, u8Len, (U8*)pu16WBuf, u8State,0);
	Driver_UnlockRes();
	free(pu8Buf);
	free(pu16WBuf);
}

void Driver_Free(U32 fd)
{
	int i;
#ifdef WIN32
	closesocket((SOCKET)fd);
#else
	close(fd);
#endif

#ifdef _Multithread_	
#ifdef WIN32
	DeleteCriticalSection(&CriticalSection);
#else
	sem_destroy(&sem_sending);
#endif
#endif
	
	for(i=0; i<6; i++)
	{
		IProtocol *plc = Protocol_GetInterface(i);
		if(plc->pdllhandle != NULL)
		{
#ifdef WIN32
#else
			dlclose(plc->pdllhandle);
#endif
		}
	}
}

typedef void (*dllentry_type)(void *);
void* Driver_InitProtocol(char *protoclModuleName)
{
	IProtocol *plc;

#ifdef WIN32
	HMODULE dllhandle;
#else
	void *dllhandle;
#endif	
	dllentry_type dllentry;	
	U32 dllpara[2];
	char pModuleFullPath[512];
	
	plc = Protocol_GetInterfaceByName(protoclModuleName);
	if(plc != NULL)
	{//the protocol has been initialized
		return plc;
	}
	//It's a new protocol. Initialize it
	plc = Protocol_GetBlank();
	
	pProtocol.Send = Driver_Send;
	pProtocol.WaitPLCResp = Protocol_WaitPLCResp;
	pProtocol.pu8Cnt = Protocol_GetU8CntPtr();
	pProtocol.pu8RxBuf = Protocol_Getpu8RxBuf();
	pProtocol.LockRes = Driver_LockRes;
	pProtocol.UnlockRes = Driver_UnlockRes;

	dllpara[0] = (int)(&pProtocol);
	dllpara[1] = (int)plc;
	//printf("driver:%x, %x\n",dllpara[0], dllpara[1]);
	
	strcpy(plc->pModuleName, protoclModuleName);
	
//	dllhandle = dlopen("./SAANet.so", RTLD_LAZY);//for SAANET
  
#ifdef WIN32
	sprintf(pModuleFullPath, "c:\\%s.dll",protoclModuleName);//printf("module:%s\n",pModuleFullPath);
  dllhandle = LoadLibraryA(pModuleFullPath);
  if(dllhandle == 0)
		printf("null handle:%s\n",pModuleFullPath);
  plc->pdllhandle = dllhandle;
  dllentry = (dllentry_type)GetProcAddress(dllhandle, "dllmain");
#else
	sprintf(pModuleFullPath, "./%s.so",protoclModuleName);//printf("module:%s\n",pModuleFullPath);
	dllhandle = dlopen(pModuleFullPath, RTLD_LAZY);//printf("11:%x\n",(int)dllhandle);//for Modbus
	
	if(dllhandle == 0)
		printf("null handle:%s\n",dlerror());
	
	plc->pdllhandle = dllhandle;
 	dllentry = (dllentry_type)dlsym(dllhandle, "dllmain");//printf("12:%x\n",(int)dllentry);
#endif 	
 	if(dllentry == 0)
 		printf("null entry\n");
 		
 	(*dllentry)(dllpara);


	return plc;
}
#ifdef WIN32
__declspec(dllexport) void __cdecl dllmain(void* para)
#else
void dllmain(void* para)
#endif  
{
	IDriver *idriver = (IDriver*)para;
	idriver->Driver_InitDriver = Driver_InitDriver;
  idriver->Driver_InitProtocol = Driver_InitProtocol;
  idriver->Driver_ReadPLC = Driver_ReadPLC;
  idriver->Driver_WritePLC = Driver_WritePLC;
  idriver->Driver_WritePLC2 = Driver_WritePLC2;
  idriver->Driver_Free = Driver_Free;
}
