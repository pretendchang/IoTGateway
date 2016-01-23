#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "../Plateform_header/typedef.h"
#include "Action.h"
#include "util.h"

typedef float ParameterType;//與勝強介面通訊所要傳遞的資料類型

class Command
{
	private:
		U32 deviceid;
		char name[128];
		ActionList *act;
		U32 u32ActCnt;
		
		void HandlingWDataFormat(U16 *para, U8 *pbuf, int datafmt);
		void HandlingRDataFormat(U32 *pKernelBuf, ParameterType *pUsrBuf, int len, int datafmt);//mapping user space mem to kernel space mem
		void GetWritingPara(char *para, U32 *inputVal, U32 *val);
		//void GetReadingPara(char *para, U32 *inputVal, U32 **val);
		void ParsingEquation(U32 *inputVal, char *equation, ParameterType *out);
		U8 ExecuteReadCommand(ActionList *actLst, U32 *inputVal, U32 **val);
		void PreParseEquation(U32 *inputVal, U32 **val, char *equation);
		
	public:
		U8 Execute(U32 *u32ValBuf);
		U8 Execute(U32 *u32ValBuf, ActionType rwtype);
		
		void SetDeviceid(int _devid){deviceid = _devid;}
		U32  GetDeviceid(){return deviceid;}
		void SetName(char *_name);
		char *GetName();
		
		ActionList *GetActionList(){return act;}
		void SetActionList(ActionList *lst){act = lst;}
		U32 GetActionCount();
		
			
};

class CommandCollection
{
	private:
		int MAX_COMMAND_CNT;
		Command *pCmndObj;
		
	public:	
		CommandCollection(){}
		void Init(char *filename);
		~CommandCollection();
		Command * GetCommand(U32 cmndid);
		U32 FindCommand(U32 _devid, char *cmndname);
		void * FindCommandObj(U32 _devid, char *cmndname);
		void *FindAllCommand(U32 _devid, U32 *len);
		int GetCollectionSize(){return MAX_COMMAND_CNT;}
		Command *operator[](int index){return (pCmndObj+index);}
};

#endif
