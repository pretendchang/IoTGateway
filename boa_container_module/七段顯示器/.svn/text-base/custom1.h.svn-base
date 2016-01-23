#ifndef _CUSTOM1_H_
#define _CUSTOM1_H_

#include "typedef.h"
#include "util.h"
#include "CPEModule.h"
typedef struct _TagObject
{
	U32 u32deviceid;//平台定義的device id
	char name[20];
	void *pTag;//指到tagsystem層的command物件
	U8 *pu8ParaBuf;//interface新定介面取得buf,由tagsystem malloc/free  暫時轉為float(ParameterType)型態，配合interface的定義
}TagObject;

class Custom1System : public CPEModule
{
	public:
		
	void* ModuleThread(void* para)
	{
		DispatchEvent();
	}
		
	void* Init(void* para)
	{
		Init();
	}
		
	void* Free(void* para)
	{
	}
		
	TagObject cmndr;
	TagObject cmndw;	
	TagObject cmndd;
	float baseValue, baseDelta;
	int baseYear, baseMonth, baseDay;	
	void DispatchEvent();
	void Init();
	void UpdateTime();
};

#endif
