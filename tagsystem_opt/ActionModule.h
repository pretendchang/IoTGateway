#ifndef _ACTIONMODULE_H_
#define _ACTIONMODULE_H_

#include "../Plateform_header/typedef.h"



struct ModuleDefaultFunction
{
		U32 u32Handle;
		U8 (*EntryPoint)(void*);
		U8 (*Init)(void* para);
	  U8 (*Free)(void* para);
	
		void (*ExecuteCommand)(U32 devid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, ActionType rwtype, U8 *u8State);
};

class ActionModule
{
	public:
	virtual U8 EntryPoint(void* para)=0;
	virtual U8 Init(void* para)=0;
	virtual U8 Free(void* para)=0;
	
	void ExecuteCommand(U32 devid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, ActionType rwtype, U8 *u8State);
};

#endif
