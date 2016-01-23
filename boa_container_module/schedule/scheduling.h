#ifndef _SCHEDULING_H_
#define _SCHEDULING_H_

#include "typedef.h"
#include "util.h"
#include "CPEModule.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

typedef struct _TagObject
{
	U32 u32deviceid;//平台定義的device id
	char name[20];
	void *pTag;//指到tagsystem層的command物件
	U8 *pu8ParaBuf;//interface新定介面取得buf,由tagsystem malloc/free  暫時轉為float(ParameterType)型態，配合interface的定義
}TagObject;
typedef struct _iENtv
{
	struct timeval _val;
}iENtv;

class SchedulingObject
{
	public:
	TagObject cmnd;//內部buf初始化20byte, 變數最多可以放10 word
	U16 pu8ScheduleTime[7];
	struct tm LastTimeStamp;  //儲存時間
	U8 u8TImecheckInterval;  // 檢查LastTimeStamp執行後, 時間經過多久才再次執行本排程
	
	int CheckSchedulingTime(struct tm *tm);
	int CheckTimeStamp(struct tm *_tm);
};

class SchedulingSystem : public CPEModule
{
	public:
	void* ModuleThread(void* para)
	{
		DispatchSchedulingEvent();
	}
		
	void* Init(void* para)
	{
		InitScheduling("./schedule.txt");
	}
		
	void* Free(void* para)
	{
		FreeScheduling();
	}
		
		
	int MAX_SchedulingObject;
	int InitScheduling(char *filename);
	SchedulingObject *pSeObj;
	void FreeScheduling();
	
	void DispatchSchedulingEvent();
	
	void ParsingSchedulingConfig(char *cc, SchedulingObject * se);
	int ReloadScheduling(char *fileName);
	private:
		 volatile int system_state;//0 for initialing, 1 for ready to run script, 2 for running script, 
};

volatile int GetSchedulingThreadState();
volatile int SetSchedulingThreadState(volatile int _val);

int TM_CheckNull(struct tm *_time);
void OSDepend_gettimeofday(iENtv *tv);
int OSDepend_GetTimeValueSec(iENtv *tv);



#endif
