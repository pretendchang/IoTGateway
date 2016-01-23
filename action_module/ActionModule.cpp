#include "ActionModule.h"
#include "stdio.h"
ActionModule *module;
U8 EntryPoint(void* para)
{
	return module->EntryPoint(para);
}

U8 Init(void* para)
{
	return module->Init(para);
}

U8 Free(void* para)
{
	return module->Free(para);
}
	
void (*ExecuteCommand)(U32 devid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, int rwtype, U8 *u8State);

void ActionModule::ExecuteCommand(U32 devid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, int rwtype, U8 *u8State)
{
	::ExecuteCommand(devid, cmndname, u32ValBuf, u8ValBufLen, rwtype, u8State);
}

void __attribute__ ((weak)) InitModuleObject()
{
}

extern "C"
{
	void dllmain(void *para)
	{
		ModuleDefaultFunction *function = (ModuleDefaultFunction *)para;
		function->EntryPoint = EntryPoint;
		function->Init = Init;
		function->Free = Free;
		ExecuteCommand = function->ExecuteCommand;
		InitModuleObject();
		
	}
}

