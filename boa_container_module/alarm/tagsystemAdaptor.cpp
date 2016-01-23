#include "tagsystemAdaptor.h"
#include "CPEModule.h"
#include <string.h>
#include <stdio.h>
int TagSystemAdaptor::SetValue(TagObject obj)
{
	return module->SetValue(obj.u32deviceid, obj.name, obj.pu8ParaBuf);
}

float *TagSystemAdaptor::GetValue(TagObject obj, U8 *u8State)
{
	float *ret = module->GetValue(obj.u32deviceid, obj.name, obj.pu8ParaBuf, u8State);
	return ret;
}


