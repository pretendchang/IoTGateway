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
	U32 u32deviceid;//���x�w�q��device id
	char name[20];
	void *pTag;//����tagsystem�h��command����
	U8 *pu8ParaBuf;//interface�s�w�������obuf,��tagsystem malloc/free  �Ȯ��ରfloat(ParameterType)���A�A�t�Xinterface���w�q
}TagObject;
typedef struct _iENtv
{
	struct timeval _val;
}iENtv;

class SchedulingObject
{
	public:
	TagObject cmnd;//����buf��l��20byte, �ܼƳ̦h�i�H��10 word
	U16 pu8ScheduleTime[7];
	struct tm LastTimeStamp;  //�x�s�ɶ�
	U8 u8TImecheckInterval;  // �ˬdLastTimeStamp�����, �ɶ��g�L�h�[�~�A�����楻�Ƶ{
	
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
