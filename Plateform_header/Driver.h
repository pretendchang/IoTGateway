#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "bool.h"
#include "typedef.h"
#include "../app_header/Node.h"

typedef struct _PLCIOCmnd
{
	U8 u8StationID;
	U16 u16LocID;
	U32 pU32Addr[4];
	U8 *pBuf;
	U8 u8Len;//in byte
	U8 *pState;//紀錄protocol層的狀態
	U32 fd;
}PLCIOCmnd;

typedef struct _ISuperProtocol//給c code用 模擬類別繼承的super指標
{
	U8 *pu8Cnt;
	U8 *pu8RxBuf;

	bool (*WaitPLCResp)();
	U32 (*Send)(U32 fd, U8 *buf, U32 nByte);
	void (*LockRes)();
	void (*UnlockRes)();
}ISuperProtocol;


//void Driver_InitDriver(U8 u8COM, U8 u8Baud, U8 u8Parity, U8 u8Data, U8 u8Stop, U8 u8NodeCount);
U32  Driver_InitDriver(CommuPara *_para);
//void Driver_SendComPort(U32 fd, U8 *pBuf, U32 nByte);
void* Driver_InitProtocol();
void Driver_SignalHandler(int signo);
void Driver_ReadPLC(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
void Driver_WritePLC(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
void Driver_WritePLC2(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, char *mask);
void Driver_Free(U32 fd);

void DegPrint(char *fmt, ...);

#endif
