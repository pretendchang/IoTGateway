
#ifndef _COMMON_H_
#define _COMMON_H_
#include  "typedef.h"
#include "util.h"
#include "CPEModule.h"
#include <stdio.h>
typedef float ParameterType ;
typedef struct _TagObject
{
	U32 u32deviceid;//���x�w�q��device id
	char name[20];
	void *pTag;//����tagsystem�h��command����
	U8 *pu8ParaBuf;//interface�s�w�������obuf,��tagsystem malloc/free  �Ȯ��ରfloat(ParameterType)���A�A�t�Xinterface���w�q
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
	char cFieldNameString[2048];//�Ĥ@��column���W�r
	arraylist pFieldFmtPara;
	
	U8 GetLogValue();
	int CheckLogTiming();

	
};

struct SysTime
{
	int time_1y;//  �褸�~
	int time_2y;//  ����~
  int time_M;//   ��
  int time_d;//   ��
  int time_w;//   �P��
  int time_1h;//  ?�� 24�p�ɨ�
  int time_m;//   ?��
  int time_s;//   ?��
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
