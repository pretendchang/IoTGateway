#ifndef _NODE_H_
#define _NODE_H_
#include "../Plateform_header/typedef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CommuPara
{
	U8 CommuTypeID;//0 for serial, 1 for ethernet
	union _CommuType
	{
		struct _serial
		{
			U8 u8COM;
			U8 u8Baud;
			U8 u8Parity;
			U8 u8Data;
			U8 u8Stop;
//			U8 u8NodeCount;
		}serial;
		
		struct _ethernet
		{
			char strIP[20];
			U16 u16Port;
//			U8 u8NodeCount;
		}ethernet;

	}CommuType;

}CommuPara;

typedef struct _NodeInfo
{
	U32 u32devid;//玡狠╰参id
	U32 u32webdevid;//呼╰参id
	U8 u8Connid;
	U8 u8Stnid;
	void *pplc;//IProtocol
	void *conn;//connection
	int *handle;//Plateform临琌ㄏノc code⊿快猭ノconnectionン璶箇痙把计ㄑcㄏノ
	CommuPara *para;
	void *idriver;//IDriver
}NodeInfo;

typedef struct _IDriver
{
	U32   (*Driver_InitDriver)(CommuPara *_para);
	void* (*Driver_InitProtocol)(char *protoclModuleName);
	void  (*Driver_SignalHandler)(int signo);
	void  (*Driver_ReadPLC)(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
	void  (*Driver_WritePLC)(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State);
	void  (*Driver_WritePLC2)(NodeInfo *pnode, char *label, U8 u8Len, U8 *pbuf, U8 *u8State, char *mask);
	void  (*Driver_Free)(U32 fd);
}IDriver;
/*
class Connection
{
	private:
		U32 u32Connid;
		U8  u8ConnType;
		char ModuleName[30];
		IDriver interface;
		int handling;
		
	public:
		CommuPara para;
		
		U32 GetConnid(){return u32Connid;}
		void SetConnid(U32 _id){u32Connid = _id;}
		void SetConnType(U8 _type){u8ConnType = _type;}
//		void SetCommuPara(CommuPara *_para){memcpy(&para, _para, sizeof(CommuPara));}
		void SetModuleName(char *name){strcpy(ModuleName, name);}
		IDriver *GetInterface(){return &interface;}
		void Establish();
};
*/
#endif
