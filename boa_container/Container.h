#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include "common.h"
class ModuleFunction
{
	public:
	char modulename[20];
	char funcname[20];
	U32 u32Handle;
	void *Execute(void *para);
};
struct ModuleDefaultFunction
{
	void* (*CPEThread)(void* para);
	void* (*Init)(void* para);
	void* (*Free)(void* para);
	void* (*Execute)(char *modulename, char *functionname, void *para);
	
	void (*ExecuteTagMultiBuf1)(U32 u32Deviceid, char *name, U8 *buf, U8 rwtype, U8 *u8State);
	void (*ExecuteTagMultiBuf2)(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State);
	float* (*GetValue)(U32 deviceid, char *name, U8 *buf, U8 *u8State);
	int (*SetValue)(U32 deviceid, char *name, U8 *buf);
};

class ModuleFunctionCollection
{
	private:
		ModuleFunction *pFunction;
		int MAX_ModuleFunctionCollection;
		static const int fieldcount=2;
	public:
		void Read(char *filename);
		
};

class CustomModule
{
	public:	
		U32 u32Handle;
		char name[20];
		
		U32 u32FunctionCnt;
		arraylist pFunction;
		ModuleDefaultFunction iInterface;
		void *Execute(char *functionname, void *para);
};

class ModuleCollection
{
	public:
		CustomModule *pModuleCol;
		int MAX_ModuleCollection;
		static const int fieldcount=2;
	//public:	
		void Read(char *filename);
		CustomModule *Find(char *name);
		void LoadModule();
		CustomModule *operator[](int index);
};

#endif
