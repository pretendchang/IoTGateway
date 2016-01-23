
#ifndef _COMMON_H_
#define _COMMON_H_
#include  "typedef.h"
#include "util.h"
#include "CPEModule.h"

//Todo: define a class inherited from CPEModule, and declare it as a child by the 'define' Macro
class CPEModuleChild : public CPEModule
{
	void* ModuleThread(void* para)
	{
	}
		
	void* Init(void* para)
	{
	}
		
	void* Free(void* para)
	{
	}
};
#endif
