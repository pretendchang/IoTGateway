
#ifndef _COMMON_H_
#define _COMMON_H_
#include  "typedef.h"
#include "util.h"
#include "CPEModule.h"
#include <stdio.h>
typedef float ParameterType ;
typedef struct _TagObject
{
	U32 u32deviceid;//平台定義的device id
	char name[20];
	void *pTag;//指到tagsystem層的command物件
	U8 *pu8ParaBuf;//interface新定介面取得buf,由tagsystem malloc/free  暫時轉為float(ParameterType)型態，配合interface的定義
}TagObject;


class LogEvent
{
	public:
	U32 u32Interval;//in sec
	char clogFileNameFmtString[256];
	arraylist plogFileNameFmtPara;
	U32 LastTimeStamp;
	TagObject cmnd;
	char cFieldFmtString[2048];
	char cFieldNameString[2048];//第一行column的名字
	arraylist pFieldFmtPara;
	
	U8 GetLogValue();
	int CheckLogTiming();

	
};

struct SysTime
{
	int time_1y;//  西元年
	int time_2y;//  民國年
  int time_M;//   月
  int time_d;//   日
  int time_w;//   星期
  int time_1h;//  ?時 24小時制
  int time_m;//   ?分
  int time_s;//   ?秒
};

//Todo: define a class inherited from CPEModule, and declare it as a child by the 'define' Macro
class LogSystem : public CPEModule
{
	public:
		SysTime timenow;
		LogEvent *LogEvt;
		int MAX_LogEvent;
		static const int fieldcount=9;
		
		void* ModuleThread(void* para)
		{
			DispatchLogEvent();
		}
		
		void* Init(void* para)
		{
			InitLog("./log.txt");
		}
		
		void* Free(void* para)
		{
			FreeLog();
		}
		
		void DispatchLogEvent();
		void InitLog(char *filename);
		void InitPara(TagObject *obj, char *para, arraylist *paraValue);
		void FreeLog();
		void UpdateSystime();
};



#endif
