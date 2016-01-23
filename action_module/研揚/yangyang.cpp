#include "ActionModule.h"
#include "yangyang.h"
#include <stdio.h>
#include <unistd.h>
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
		int i;
	U8 u8state=0;
	float curr, on=1.0;
	float val = *(float*)para;
	ExecuteCommand(1, "rmanual", (U32*)&curr, 4, 0, &u8state);
	if(u8state != 1)
		return u8state;
	if(curr!=1.0)
		return 1;
		
	ExecuteCommand(1, "setemp", (U32*)&curr, 4, 0, &u8state);
	
	if(u8state != 1)
		return u8state;
	
	float diff = val-curr;printf("val:%f %f\n",val,curr);
	int idiff = (int)(diff/0.5);
	printf("diff:%d\n",idiff);
	if(idiff > 0)
	{
		for(i=0; i<idiff; i++)
		{
			ExecuteCommand(1, "uptemp", (U32*)&on, 4, 1, &u8state);
			if(u8state != 1)
				return u8state;
			if((i+1)<idiff)
				sleep(1);	
		}
	}
	else if(idiff < 0)
	{
		for(i=0; i<(-idiff); i++)
		{
			ExecuteCommand(1, "downtemp", (U32*)&on, 4, 1, &u8state);
			if(u8state != 1)
				return u8state;

			if((i+1)<(-idiff))
				sleep(1);	
		}
	}
	return 1;		
}

U8 yangyangActionModule::Init(void* para)
{
	printf("yangyangActionModule init\n");
}

U8 yangyangActionModule::Free(void* para)
{
}
