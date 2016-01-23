#ifndef _ACTION_H_
#define _ACTION_H_

#include "../Plateform_header/typedef.h"
#include "util.h"


typedef enum _ActionType
{
	READ = 0,
	WRITE,
	BATCH_WRITE_BIT,
	DELAY,
	MODULE
}ActionType;

typedef enum _ActionDataFormat
{
	TYPE_UINT = 0,	//16bit
	TYPE_INT,				//16bit
	TYPE_BOOL,
	TYPE_FLOAT,			//32bit
	TYPE_BCD,				//32bit
	TYPE_UINT8,
	TYPE_UINT32,
	TYPE_INT8,
	TYPE_INT32
}ActionDataFormat;
#include "ActionModule.h"
class Action
{
	private:
		void GetWritingMask(char *para, char *mask);
		
		
		
	public:
		U32 deviceid;
		char tagname[64];
		char label[64];
		U8 *pu8ActBuf;
		ActionType rw;//0:for read  1: for write 2:special write, read before write f denote default value, 1 for on, 0 for off
		U32 len;//in byte
		ActionDataFormat datafmt;//format for PLC response data  0:uint 1:int 2:bool 3:float
		U8 *pu8State;//紀錄讀取狀態 read command用
		
		ModuleDefaultFunction pModule;//當rw=4 module模式時 紀錄module的handle
		void InitModule();
		
		
		Action();
		~Action();
//		U8 Execute(U32 *u32ValBuf);
		U8 Execute(U32 *u32ValBuf, ActionType rwtype);
};

class ActionList
{
	private:
		Action *pAct;
		ActionList *pNext;
		
	public:
		char para[64];
		//pattern (...)  ... is the parameter list
		//In decimal
		//@tag denotes for the para from external system (scheduling system or plateform controller).
		//@1 for 1st para, @2 for 2nd para, etc..
		//It can be a integer constant. ex: 1, 2, 3...   Bit value can be 0 or 1
		//It can be a equation. Start with '='  ex:=@1*512+25600 
		
		void AddAction(Action *_pact, char *para);
		~ActionList();
		
		ActionList *GetNext(){return pNext;}
		void SetAction(Action *_pact){pAct = _pact;}
		void SetNext(ActionList *_next){pNext = _next;}
		Action *GetAction(){return pAct;}
};

class ActionCollection
{
	private:
		Action *pActObj;

	public:
		int MAX_ACTION_CNT;
		ActionCollection(){}
		void Init(char *filename);
		
		~ActionCollection();
		
		int FindAction(U32 _devid, char *tagname);
		Action * GetAction(U32 actid);
};

#endif
