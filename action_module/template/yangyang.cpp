#include "ActionModule.h"
#include "yangyang.h"
#include <stdio.h>
extern ActionModule *module;

void InitModuleObject()
{
	module = new yangyangActionModule();
}
/*
 *ExecuteCommand manual note
 *1st web device id
 *2nd tag name
 *3rd IO buf
 *4th IO buf length. Unit is in bytes.
 *5th action type refer to config
 *6th action state
*/
U8 yangyangActionModule::EntryPoint(void* para)
{
	printf("yangyangActionModule entry point\n");
	float a=0;
	U8 state=0;
	ExecuteCommand(1, "R2", (U32*)&a, 4, 0, &state);//對外的溝通都是float
	printf("ret:%f\n",a); 
	return state;
}

U8 yangyangActionModule::Init(void* para)
{
	printf("yangyangActionModule init\n");
}

U8 yangyangActionModule::Free(void* para)
{
}
