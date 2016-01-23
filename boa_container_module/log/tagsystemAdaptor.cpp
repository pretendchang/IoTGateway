#include "tagsystemAdaptor.h"

extern void (*ExecuteTagMultiBuf2)(U32 u32Deviceid, char *name, U8 *buf, U8 *u8State);
void TagSystemAdaptor::ExecuteLog(TagObject *obj, U8 *u8State)
{
	::ExecuteTagMultiBuf2(obj->u32deviceid, obj->name, obj->pu8ParaBuf, u8State);
}

