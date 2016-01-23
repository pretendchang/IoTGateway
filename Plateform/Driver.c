#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#include <pthread.h>

#include "../Plateform_header/Driver.h"
#include "../Plateform_header/Protocol.h"

#define SIGTIMER (SIGRTMAX)
char *pwCOM[4] = {"/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3"};
int  iBaud[7] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400};
struct sigaction saio;           /* definition of signal action */
//int fd;
U32 currFD;
ISuperProtocol pProtocol;
IProtocol *currplc;

U32 Driver_SendComPort(U32 fd, U8 *pBuf, U32 nByte)
{
	int i=0,j=0,res;char buf[255];
	volatile U8 *pRxStatus;
	
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec=2;
	tv.tv_usec=0;
//	usleep(10000);
	DegPrint("send:");
	for(i=0; i<nByte; i++)
	{
		DegPrint("%x, ",pBuf[i]);
	}DegPrint("\n");
	write(fd,pBuf,nByte);
	pRxStatus = GetRxStatus();
	*pRxStatus=0;
	
	while((*pRxStatus)==0)
	{
		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if(retval == -1)
			printf("err\n");
		else if(FD_ISSET(fd, &rfds))
		{
			res = read(fd, buf, 255);
			if(res>0)
			{
				for(i=0; i<res; i++)
				{
				  Protocol_CommuISR(currplc, buf[i]);
				}
			}
			j++;
			if(j>2000)
			{//timeout
				printf("timeout\n");
				return 2;
			}
		}
		else
		{//timeout
			printf("timeout\n");
			return 2;
		}
	}
}

#ifdef _Multithread_
#include<semaphore.h>
sem_t sem_sending;
volatile pthread_cond_t condGetOK = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexGetOKLock = PTHREAD_MUTEX_INITIALIZER;
#endif

U32 Driver_InitDriver(CommuPara *pPara)
{
	int c, res,i;
  struct termios oldtio,newtio;
  U32 fd;
  
  char buf[255]; 

  /* 開啟裝置為 non-blocking (讀取功能會馬上結束返回) */
  fd = open(pwCOM[pPara->CommuType.serial.u8COM], O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd <0) {perror(pwCOM[pPara->CommuType.serial.u8COM]); exit(-1); }
  
  
  /* 允許行程去接收 SIGIO 訊號*/
  //fcntl(fd, F_SETOWN, getpid());
  /* 使檔案ake the file descriptor 非同步 (使用手冊上說只有 O_APPEND 及
  O_NONBLOCK, 而 F_SETFL 也可以用...) */
  //fcntl(fd, F_SETFL, FASYNC);

  tcgetattr(fd,&oldtio); /* 儲存目前的序列埠設定值 */
  /* 設定新的序列埠為標準輸入程序 */
  newtio.c_cflag = CRTSCTS | CLOCAL | CREAD;
  newtio.c_cflag |= iBaud[pPara->CommuType.serial.u8Baud];
  switch(pPara->CommuType.serial.u8Parity)
  {
  	case 0://none
  		newtio.c_cflag &= ~PARENB;
  		break;
  	
  	case 1://even
  		newtio.c_cflag |= PARENB; 
  		newtio.c_cflag &= ~PARODD;
  		break;
  		
    case 2://odd
  		newtio.c_cflag |= PARENB; 
  		newtio.c_cflag |= PARODD;		
	}
	
	switch(pPara->CommuType.serial.u8Stop)
	{
		case 0://1
			newtio.c_cflag &= ~CSTOPB;
			break;
		case 1://2
			newtio.c_cflag |= CSTOPB;
			break;	
	}
	
	switch(pPara->CommuType.serial.u8Data)
	{
		case 0://7
			newtio.c_cflag &= ~CSIZE;
			newtio.c_cflag |= CS7;
			break;
			
		case 1://8
			newtio.c_cflag &= ~CSIZE;
			newtio.c_cflag |= CS8;
			break;	
	}
  
  
  
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  newtio.c_lflag=0;
  newtio.c_cc[VMIN]=1;//讀幾個char回來後觸發
  newtio.c_cc[VTIME]=5;//500ms後沒收到新的char, timeout, read會return
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
  
#ifdef _Multithread_	
	sem_init(&sem_sending, 0, 1);
#endif	
	
	return fd;
  
}

void Driver_LockRes()
{
#ifdef _Multithread_
	int a=sem_wait(&sem_sending);
#endif	
}

void Driver_UnlockRes()
{
#ifdef _Multithread_	
	int b=sem_post(&sem_sending);
#endif	
}

//u8Len in byte
//*u8State 1: success  2:err
//U8 u8Sync 1:activate Semaphore mechanism  0:disable Semaphore mechanism
void Driver_ReadPLC_Core(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, U8 u8Sync)
{
	U8 buf[255];//放cmnd轉出plc command的結果
	U8 u8Txlen = 0;
	PLCIOCmnd cmnd;
	int _res;
	U8 u8ProtocolState = 0;
	IProtocol *plc;
	U8 *pu8Cnt = Protocol_GetU8CntPtr();
	U8 u8StnID = pnode->u8Stnid;
	
	cmnd.pBuf = pbuf;
	cmnd.pState = &u8ProtocolState;
	cmnd.fd=*(pnode->handle);
	
	if(u8Sync == 1)
		Driver_LockRes();
	
	plc = (IProtocol*)pnode->pplc;
	currFD = *(pnode->handle);
	currplc = plc;
	plc->Label2Cmnd(label, u8StnID, u8Len, &cmnd);
	plc->GenReadTelegram(&cmnd, buf, &u8Txlen);

	Driver_SendComPort(*(pnode->handle), buf,u8Txlen);

	if(Protocol_WaitPLCResp())
	{//ok
		plc->ParseReadTelegram(&cmnd);
		*pu8Cnt = 0;
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

void Driver_WritePLC_Core(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, U8 u8Sync)
{
	U8 buf[255];//放cmnd轉出plc command的結果
	U8 u8Txlen = 0;
	PLCIOCmnd cmnd;
	int _res;
	U8 u8ProtocolState = 0;
	U8 u8StnID = pnode->u8Stnid;
	
	cmnd.pBuf = pbuf;
	cmnd.pState = &u8ProtocolState;
	cmnd.fd=*(pnode->handle);
	IProtocol *plc;
	
	if(u8Sync == 1)
		Driver_LockRes();

	plc = pnode->pplc;
	currFD = *(pnode->handle);
	currplc = plc;
	plc->Label2Cmnd(label, u8StnID, u8Len, &cmnd);
	plc->GenWriteTelegram(&cmnd, buf, &u8Txlen);

	Driver_SendComPort(*(pnode->handle), buf,u8Txlen);
//	tid = SetTimer(SIGTIMER, 400, 1);//Protocol RX timeout timer
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
	U8 *pu8Buf = (U8*)malloc(u8Len);memset(pu8Buf, 0, u8Len);
	U16 *pu16WBuf = (U16*)malloc(u8Len);memset(pu16WBuf, 0, u8Len);
	int i;
	
	Driver_LockRes();
	Driver_ReadPLC_Core(pnode, label, u8Len, pu8Buf, u8State, 0);
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
#include <dlfcn.h>
void Driver_Free(U32 fd)
{
	int i;
//	free(pNodeObj);
	close(fd);
	
#ifdef _Multithread_	
	sem_destroy(&sem_sending);
#endif
	
	for(i=0; i<6; i++)
	{
		IProtocol *plc = Protocol_GetInterface(i);
		if(plc->pdllhandle != NULL)
			dlclose(plc->pdllhandle);
	}
}
typedef void (*dllentry_type)(void *);
void* Driver_InitProtocol(char *protoclModuleName)
{
	void *dllhandle;
	dllentry_type dllentry;
	U32 dllpara[2];
	char pModuleFullPath[512];
	
	IProtocol *plc = Protocol_GetInterfaceByName(protoclModuleName);
	if(plc != NULL)
	{//the protocol has been initialized
		return plc;
	}
	//It's a new protocol. Initialize it
	plc = Protocol_GetBlank();
	
	pProtocol.Send = Driver_SendComPort;
	pProtocol.WaitPLCResp = Protocol_WaitPLCResp;
	pProtocol.pu8Cnt = Protocol_GetU8CntPtr();
	pProtocol.pu8RxBuf = Protocol_Getpu8RxBuf();

	dllpara[0] = (int)(&pProtocol);
	dllpara[1] = (int)plc;
//	printf("driver:%x, %x\n",dllpara[0], dllpara[1]);
	
	strcpy(plc->pModuleName, protoclModuleName);
	
//	dllhandle = dlopen("./SAANet.so", RTLD_LAZY);//for SAANET
  sprintf(pModuleFullPath, "./%s.so",protoclModuleName);//printf("module:%s\n",pModuleFullPath);
	dllhandle = dlopen(pModuleFullPath, RTLD_LAZY);//printf("11:%x\n",(int)dllhandle);//for Modbus
	plc->pdllhandle = dllhandle;
 	dllentry = (dllentry_type)dlsym(dllhandle, "dllmain");//printf("12:%x\n",(int)dllentry);
 	(*dllentry)(dllpara);
// 	dlclose(dllhandle);	
	return plc;
}

void dllmain(void* para)//first dword:  
{
	IDriver *interface = (IDriver*)para;
	interface->Driver_InitDriver = Driver_InitDriver;
  interface->Driver_InitProtocol = Driver_InitProtocol;
  interface->Driver_ReadPLC = Driver_ReadPLC;
  interface->Driver_WritePLC = Driver_WritePLC;
  interface->Driver_WritePLC2 = Driver_WritePLC2;
  interface->Driver_Free = Driver_Free;
//  interface->Driver_SignalHandler = Driver_RxThread;//Driver_SignalHandler;
}


