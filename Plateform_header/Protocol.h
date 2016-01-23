#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "bool.h"
#include "typedef.h"
#include "Driver.h"


typedef struct _IProtocol
{
	char pModuleName[64];
	void *pdllhandle;
	void (*Label2Cmnd)(char* pLabel, U8 u8StnID, U8 u8Len, PLCIOCmnd *pCmnd);//u8Len in byte
	void (*GenReadTelegram)(PLCIOCmnd *pCmnd, U8 *pbuf, U8 *u8Txlen);
	void (*ParseReadTelegram)(PLCIOCmnd * cmnd);
	void (*GenWriteTelegram)(PLCIOCmnd *pCmnd, U8 *pBuf, U8 *u8Txlen);
	void (*ParseWriteTelegram)(PLCIOCmnd * cmnd);
	bool (*CheckRespEnd)(void);
	void (*ReadPLC)(int handle, U8 u8StnID, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
	void (*WritePLC)(int handle, U8 u8StnID, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
}IProtocol;

bool Protocol_WaitPLCResp(void);
void Protocol_CommuISR(IProtocol *plc, U8 c);
IProtocol *Protocol_GetInterface(int i);
U8 *Protocol_GetU8CntPtr();
U8 *Protocol_Getpu8RxBuf();
IProtocol *Protocol_GetBlank();
IProtocol *Protocol_GetInterfaceByName(char *modulename);
volatile U8 * GetRxStatus();

#endif
