#include "../Plateform_header/Protocol.h"

#define RxBufMaxLen 512
volatile bool bGetOK;
U8 u8Cnt = 0;
U8 pu8RxBuf[RxBufMaxLen];

U32 pInterfaceIdx=0;
#define IProtocolMaxCnt 6
IProtocol pInterface[IProtocolMaxCnt];

volatile U8 * GetRxStatus()
{
	return &bGetOK;
}

IProtocol *Protocol_GetInterface(int i)
{
	return &pInterface[i];
}

IProtocol *Protocol_GetInterfaceByName(char *modulename)
{
	int i;
	for(i=0; i<IProtocolMaxCnt; i++)
	{
		if(strcmp(modulename, pInterface[i].pModuleName) == 0)
		{
//			printf("%d %x\n",i,(int)&pInterface[i]);
			return &pInterface[i];
		}
	}//printf("find nothing\n");
	return 0;
}

IProtocol *Protocol_GetBlank()
{
//	printf("blank:%d %x\n",pInterfaceIdx, &pInterface[pInterfaceIdx]);
	return &pInterface[pInterfaceIdx++];
}

#ifdef _Multithread_
#include<semaphore.h>
#include<time.h>
extern volatile pthread_cond_t condGetOK;
extern pthread_mutex_t mutexGetOKLock;
#endif
bool Protocol_WaitPLCResp()
{
	if(bGetOK)
	{
			//printf("ok\n");
			return true;
	}
	else
	{
			printf("timeout\n");
			u8Cnt = 0;
			bGetOK = false;
			return false;
	}
}

void Protocol_CommuISR(IProtocol *plc, U8 c)
{
	pu8RxBuf[u8Cnt++] = c;
	DegPrint("t(%d):%x\n",u8Cnt, c);
	if(plc->CheckRespEnd())
	{//printf("Rok\n");
		bGetOK = true;
		u8Cnt = 0;
		//pthread_cond_signal(&condGetOK);
	}
	else
	{//printf("Rnok\n");
		bGetOK = false;
	}
}

U8 *Protocol_GetU8CntPtr()
{
	return &u8Cnt;
}

U8 *Protocol_Getpu8RxBuf()
{
	return pu8RxBuf;
}
