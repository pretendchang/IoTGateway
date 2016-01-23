#ifndef _CPEMODULE_H_
#define _CPEMODULE_H_

#include "typedef.h"
//#include "common.h"

extern void* (*Execute)(char *modulename, char *functionname, void* para);
extern void (*ExecuteTagMultiBuf1)(U32 u32Deviceid, char *name, U8 *buf, U8 rwtype, U8 *u8State);
extern void (*ExecuteTagMultiBuf2)(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State);
extern float* (*GetValue)(U32 deviceid, char *name, U8 *buf, U8 *u8State);
extern int (*SetValue)(U32 deviceid, char *name, U8 *buf);

class CPEModule
{
	public:
	virtual void* ModuleThread(void* para)=0;
	virtual void* Init(void* para)=0;
	virtual void* Free(void* para)=0;
	
	void* Execute(char *modulename, char *functionname, void* para)
	{
		return ::Execute(modulename, functionname, para);
	}
	
	void ExecuteTagMultiBuf1(U32 u32Deviceid, char *name, U8 *buf, U8 rwtype, U8 *u8State)
	{
		return ::ExecuteTagMultiBuf1(u32Deviceid, name, buf, rwtype, u8State);
	}
	
	void ExecuteTagMultiBuf2(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State)
	{
		return ::ExecuteTagMultiBuf2(u32Deviceid, name, buf, u8State);
	}
	
	float* GetValue(U32 deviceid, char *name, U8 *buf, U8 *u8State)
	{//printf("%d %s\n",deviceid,name);
		return ::GetValue(deviceid, name, buf, u8State);
	}

	int SetValue(U32 deviceid, char *name, U8 *buf)
	{
		return ::SetValue(deviceid, name, buf);
	}
};

extern CPEModule *module;
#endif
