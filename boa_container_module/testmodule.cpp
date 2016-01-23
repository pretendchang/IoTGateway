
#include <stdio.h>
#include <unistd.h>

#include "CPEModule.h"

CPEModule *module;
struct ModuleDefaultFunction
{
	void* (*CPEThread)(void* para);
	void* (*Init)(void* para);
	void* (*Free)(void* para);
	void* (*Execute)(char *modulename, char *functionname, void* para);
	
	void (*ExecuteTagMultiBuf1)(U32 u32Deviceid, char *name, U8 *buf, U8 rwtype, U8 *u8State);
	void (*ExecuteTagMultiBuf2)(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State);
	float* (*GetValue)(U32 deviceid, char *name, U8 *buf, U8 *u8State);
	int (*SetValue)(U32 deviceid, char *name, U8 *buf);
};



void* (*Execute)(char *modulename, char *functionname, void* para);
void (*ExecuteTagMultiBuf1)(U32 u32Deviceid, char *name, U8 *buf, U8 rwtype, U8 *u8State);
void (*ExecuteTagMultiBuf2)(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State);
float* (*GetValue)(U32 deviceid, char *name, U8 *buf, U8 *u8State);
int (*SetValue)(U32 deviceid, char *name, U8 *buf);

void* CPEThread(void* para)
{
	while(1)
	{
		module->ModuleThread(0);
		usleep(100000);
	}
}

void* Init(void* para)
{
	printf("Init\n");
	module->Init(0);
}

void* Free(void* para)
{
	printf("Free\n");
	module->Free(0);
}

int main()
{
	Init(0);
	while(1)
	{
		CPEThread(0);
	}
}

void __attribute__ ((weak)) InitModuleObject()
{
}
extern "C"
{
	void dllmain(void* para)//first dword:  
	{
		ModuleDefaultFunction *pinterface = (ModuleDefaultFunction*)para;
		pinterface->CPEThread = CPEThread;
	  pinterface->Init = Init;
	  pinterface->Free = Free;
	  Execute = pinterface->Execute;
	  ExecuteTagMultiBuf1 = pinterface->ExecuteTagMultiBuf1;
	  ExecuteTagMultiBuf2 = pinterface->ExecuteTagMultiBuf2;
	  GetValue = pinterface->GetValue;
	  SetValue = pinterface->SetValue;
	  InitModuleObject();
	}
	
	void *cus(void *para)
	{
		printf("exp fun\n");
	}
}
