
#ifndef _COMMON_H_
#define _COMMON_H_
#include  "typedef.h"
#include "util.h"
#include "CPEModule.h"

//Todo: define a class inherited from CPEModule, and declare it as a child by the 'define' Macro
class CPEModuleChild : public CPEModule
{
	int i;
	void* ModuleThread(void* para)
	{
		printf("i:%d\n",i++);
	}
		
	void* Init(void* para)
	{
		i=0;
	}
		
	void* Free(void* para)
	{
	}
};
#endif
