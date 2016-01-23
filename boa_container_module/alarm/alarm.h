#ifndef _ALARM_H_
#define _ALARM_H_

#include "typedef.h"
#include "util.h"
#include "equation.h"
#include "CPEModule.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

typedef struct _TagObject
{
	U32 u32deviceid;//平台定義的device id
	char name[20];
	void *pTag;//指到tagsystem層的command物件
	U8 *pu8ParaBuf;//interface新定介面取得buf,由tagsystem malloc/free  暫時轉為float(ParameterType)型態，配合interface的定義
}TagObject;

class RuleExt
{
	public:
	char name[50];
	U8 u8Ruldidx;
	char oid[50];
	U8 u8AckType;
	U32 u32NotifyTimes;
};

class RuleExtCol
{
	private:
		int MAX_RuleExtCol;
		
	public:
	static const int fieldcount=5;
	RuleExt *pRuleExtCol;
	RuleExt *FindRuleExt(char *name, U8 idx);
	void Init(char *filename);
	~RuleExtCol();
};

class Rule
{
	public:
	Condition cond;
	TagObject cmnd[2];//多個物件
	int iCmndCount;
	char msg[50];
	char *sOidIp;
	
	int state;
	int firecount;
	U32 u32LastFireTimeStamp;
	
	
	RuleExt *ext;
	
	Rule *next;
	Rule *prev;
	
	U8 GetAlarmValue(U8 *pu8State);
	void Execute();
	Rule(){state=0;firecount=0;cmnd[0].pTag=0;cmnd[0].name[0]=0;cmnd[1].pTag=0;cmnd[1].name[0]=0;next=0;prev=0;u32LastFireTimeStamp=0;}
	~Rule();
};



struct datasnmp
{//cqi格式介面
	char name[50];
	char ip1[20];
	char ip2[20];
	char ip3[20];
	char ip4[20];
	char ruletype[20];
	char interval[20];
	char delay[20];
	char starttime[20];
	char stoptime[20];
	char mainvalue[20];
	char mainThreshold1[20];
	char mainThreshold2[20];
	char mainThreshold3[20];
	char concern1[20];
	char concern2[20];
	char concern3[20];
	char pairValue1[20];
	char pairValue2[20];
	char pairValue3[20];
	char pairThreshold1[20];
	char pairThreshold2[20];
	char pairThreshold3[20];
	char msg1[50];
	char msg2[50];
	char msg3[50];
	char oid[50];
	char acktype[20];
	char notifytimes[20];
};

// Alarm  結構
class AlarmContent
{
	public:	
	Rule rule;
	
	char sPublicIp[30];
	U8   u8Enable;          // 1 啟用,0 停止
	char AlarmName[30];		     // ID
	U8 pu8StartTime[7];//schedule system crontable 格式  cqi此欄位隱含daily排程，因此pu8StartTime[0]=2
	U8 pu8StopTime[7];//schedule system crontable 格式 cqi此欄位隱含daily排程，因此pu8StartTime[0]=2
//	U32  u32StartTime;
//	U32  u32StopTime;
	U32   u32Freq;		         // 頻率(Sec)
	U32  u32Delay;
	
	void Execute();
	void Init(datasnmp data);
	~AlarmContent();
	AlarmContent(){rule.prev=NULL;memset(pu8StartTime, 0xff ,8);memset(pu8StopTime, 0xff ,8);LastTimeStamp=0;}
	
	U32 LastTimeStamp;
	
	int CheckAlarmTiming();
	
	
	void CheckExecuteSession(Rule *r);
	
	
	private:
	
	//cqi格式轉換函式	
	void Threshold2Rule(U8 idx, char *mainThreshold, char *concern, char *pairThreshold, Rule *r);
	void datasnmp2Condition(datasnmp data);
	void ParseThreshold(char *sThreshold, char *out1, char *out2);
	void ParseValue(char *sValue, char *out1, char *out2);
	void ParseTime(char *sTime, char *out1, char *out2);
	
	
};

class AlarmSystem : public CPEModule
{
	public:
	void* ModuleThread(void* para)
	{
		DispatchAlarmEvent();
	}
		
	void* Init(void* para)
	{
		InitAlarm("./alarm.txt", "./alarmext.txt");
	}
		
	void* Free(void* para)
	{
		FreeAlarm();
	}
		
		
	AlarmContent *pAlarmCol;
	RuleExtCol ruleextcol;
	U32 Max_AlarmCnt;

	
	void DispatchAlarmEvent();
	void InitAlarm(char *filename, char *extfilename);
	void getValue(char fileline, char *token, int *tokenidx, datasnmp *file, int *strstate);
	
	void InitAlarmContent();
	void CloseUDPCommu();
	void SystemAlarm(U32 u32Deviceid, U32 u32AlarmID);

	void FreeAlarm();
	
	void ParseAlarmContent(AlarmContent *palmcnt, char *alarmcnt);
	int ReloadAlarmContent(char *fileName);
	~AlarmSystem(){delete []pAlarmCol;}
	
};

typedef enum _SysAlarm
{
	SYSALM_PLC_Timeout = 0,
	SYSALM_CFGFile_Err
}SysAlarm;


class FireOID
{
	public:
	//static int sockfd;
	static void OIDStr2OID(char *oidstr, U8 *oid, U8 *oidlen);
	static void DecodeOID(char *oidstr, U8 *oid, U8 *oidlen);
	static void SendSNMPTrap(int *sockfd, char *oid, char *msg);
	static void InitUDPCommu(int *sockfd, char *strip);
	static void CloseUDPCommu(int *sockfd);
};


#endif
