#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "scheduling.h"


void InitModuleObject()
{
	module = new SchedulingSystem();
}

int SchedulingObject::CheckTimeStamp(struct tm *_tm)
{
	//printf("%d %d %d %d %d %d\n",_tm->tm_yday,_tm->tm_hour,_tm->tm_min,LastTimeStamp.tm_yday,LastTimeStamp.tm_hour,LastTimeStamp.tm_min);
	switch(u8TImecheckInterval)
	{
		case 3://min
			if(_tm->tm_yday == LastTimeStamp.tm_yday &&
				 _tm->tm_hour == LastTimeStamp.tm_hour &&
				 _tm->tm_min  == LastTimeStamp.tm_min)
				 return 1;//執行過
			else
				return 0;
			
			break;
	}
	return 1;//有問題, 回覆已執行過, 避免該排程被執行
}

/*檢查 週排程  */
int SchedulingObject::CheckSchedulingTime(struct tm *tm)
{ 	
		int week;
		switch(pu8ScheduleTime[0])
		{ 
			case 0:  // Monthly
				
				//printf("1set: %d %d %d %d %d \n",pu8ScheduleTime[1],pu8ScheduleTime[2], pu8ScheduleTime[3], pu8ScheduleTime[5], pu8ScheduleTime[6]);
				//printf("1now: %d %d %d %d %d \n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday ,tm->tm_hour, tm->tm_min, tm->tm_sec);
	
				if(pu8ScheduleTime[3]==tm->tm_mday &&  pu8ScheduleTime[5]==tm->tm_hour && pu8ScheduleTime[6]==tm->tm_min )
				{
					//printf("月排程 \n");
				 	if(TM_CheckNull(&(LastTimeStamp)) == 0)
					{//time object isn't a null object. We have to check the last time stamp
						if(CheckTimeStamp(tm))
						{
							//排程在設定的檢查區間內, 跑過
							return true;//此次不用執行排程
						}
						else
						{
							//排程在設定的檢查區間內, 還沒跑過
							memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
							return false;//需要執行
						}
						
					}
					else
					{
						//time object is a null object. This is the first time the scheduling event is executed.
						//save time stamp
						memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
						
						return false;//需要執行
					}
				}
				 
				return true;				
				break;
			case 1:   //weekly				
				
				switch (tm->tm_wday)
				{
					case 0:
						week=7;
						break;
					default:
						week=tm->tm_wday;
						break;
				}
				//printf("set:%d %d %d\n", pu8ScheduleTime[4], pu8ScheduleTime[5], pu8ScheduleTime[6]);
				//printf("now:%d %d %d %d\n",week, tm->tm_hour, tm->tm_min, tm->tm_sec);
	
				if(pu8ScheduleTime[4]==(week) && pu8ScheduleTime[5]==tm->tm_hour && pu8ScheduleTime[6]==tm->tm_min)
				{ 
				 	if(TM_CheckNull(&(LastTimeStamp)) == 0)
					{//time object isn't a null object. We have to check the last time stamp
						if(CheckTimeStamp(tm))
						{
							//排程在設定的檢查區間內, 跑過
							return true;//此次不用執行排程
						}
						else
						{
							//排程在設定的檢查區間內, 還沒跑過
							memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
							return false;//需要執行
						}
						
					}
					else
					{
						//time object is a null object. This is the first time the scheduling event is executed.
						//save time stamp
						memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
						
						return false;//需要執行
					}
				}	
				//else 
					//printf("二者不合\n");			 
				return true;
				break;
			case 2:  // daily
				//printf("set: %d %d %d %d %d \n",pu8ScheduleTime[1],pu8ScheduleTime[2], pu8ScheduleTime[3], pu8ScheduleTime[5], pu8ScheduleTime[6]);
				//printf("now: %d %d %d %d %d \n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday ,tm->tm_hour, tm->tm_min, tm->tm_sec);
	
				if(pu8ScheduleTime[5]==tm->tm_hour && pu8ScheduleTime[6]==tm->tm_min)
				{
					//printf("日排程 \n");
				 	if(TM_CheckNull(&(LastTimeStamp)) == 0)
					{//time object isn't a null object. We have to check the last time stamp
						if(CheckTimeStamp(tm))
						{
							//排程在設定的檢查區間內, 跑過
							return true;//此次不用執行排程
						}
						else
						{
							//排程在設定的檢查區間內, 還沒跑過
							memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
							return false;//需要執行
						}
						
					}
					else
					{
						//time object is a null object. This is the first time the scheduling event is executed.
						//save time stamp
						memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
						
						return false;//需要執行
					}
				}
				return true;				
				break;
			case 3:	 // once
				//printf("set: %d %d %d %d %d \n",pu8ScheduleTime[1],pu8ScheduleTime[2], pu8ScheduleTime[3], pu8ScheduleTime[5], pu8ScheduleTime[6]);
				//printf("now: %d %d %d %d %d \n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday ,tm->tm_hour, tm->tm_min, tm->tm_sec);
	
				if(pu8ScheduleTime[1]==(tm->tm_year+1900) && pu8ScheduleTime[2]==(tm->tm_mon+1) && pu8ScheduleTime[3]==tm->tm_mday &&  pu8ScheduleTime[5]==tm->tm_hour && pu8ScheduleTime[6]==tm->tm_min )
				// if(pSecheduling[ScIndex].pu8ScheduleTime[4]==(tm->tm_wday) && pSecheduling[ScIndex].pu8ScheduleTime[5]==tm->tm_hour && pSecheduling[ScIndex].pu8ScheduleTime[6]==tm->tm_min)
				{
					//printf("一次性排程 \n");
				 	if(TM_CheckNull(&(LastTimeStamp)) == 0)
					{//time object isn't a null object. We have to check the last time stamp
						if(CheckTimeStamp(tm))
						{
							//排程在設定的檢查區間內, 跑過
							return true;//此次不用執行排程
						}
						else
						{
							//排程在設定的檢查區間內, 還沒跑過
							memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
							return false;//需要執行
						}
						
					}
					else
					{
						//time object is a null object. This is the first time the scheduling event is executed.
						//save time stamp
						memcpy(&(LastTimeStamp), tm, sizeof(struct tm));
						
						return false;//需要執行
					}
				}
				return true;
				break;
		}
}


// 執行排程 Event
void SchedulingSystem::DispatchSchedulingEvent()
{
//	Device *pd;
//	Command *pc;
	U32 i, j, devid;
	U16 u16ValBuf;
	iENtv tv;
	time_t t;
	struct tm tm;
	
	U8 u8State;
			
	OSDepend_gettimeofday(&tv);  // 取得日期
	t = OSDepend_GetTimeValueSec(&tv);
	memcpy(&tm, localtime(&t), sizeof(struct tm));
	
	for(i=0; i<MAX_SchedulingObject&&system_state==1; i++)
	{
		// 檢查是否到達執行時間
		if(pSeObj[i].CheckSchedulingTime(&tm))
		{
			//還沒到執行時間, 執行下一個shceduling script
			continue;
		}
		if(system_state != 1)
			break;
			
		system_state = 2;
		printf("Run (%d) Scheduling Type[%d] Time ::  %d/%d/%d Week [%d] %d:%d \n",
						i,pSeObj[i].pu8ScheduleTime[0],pSeObj[i].pu8ScheduleTime[1],
						pSeObj[i].pu8ScheduleTime[2],pSeObj[i].pu8ScheduleTime[3]
						,pSeObj[i].pu8ScheduleTime[4],pSeObj[i].pu8ScheduleTime[5],pSeObj[i].pu8ScheduleTime[6]);
						printf("command:%d %s %f\n",pSeObj[i].cmnd.u32deviceid, pSeObj[i].cmnd.name, *(float*)pSeObj[i].cmnd.pu8ParaBuf);
		ExecuteTagMultiBuf2(pSeObj[i].cmnd.u32deviceid, pSeObj[i].cmnd.name, pSeObj[i].cmnd.pu8ParaBuf, &u8State);
		//TagSystemAdaptor::ExecuteSchedule(pSeObj[i].cmnd, &u8State);

		if ( u8State != 1)
		{
				printf("fail\n");
		}
		system_state=1;
	}
}


// parsing 字串
void SchedulingSystem::ParsingSchedulingConfig(char *cc, SchedulingObject * se)
{
	char *firstLargeStart, *firstLargeEnd, *secondLargeStart, *secondLargeEnd;
	char freq[10];  				// 排程週期
	char firstLarge[100];
	char secondLarge[100];
	char tmpstr1[10],tmpstr2[10],tmpstr3[10],tmpstr4[10],tmpstr5[10],tmpstr6[10],tmpstr7[10],tmpstr8[10],tmpstr9[10];
	int tmval1=0,tmval2=0,tmval3=0,tmval4=0,tmval5=0,tmval6=0,devid=0;
	char *tmp, *prevtmp;
	U32 cmndid;

  float fValue;
  
	firstLargeStart = strchr(cc, '{');
	firstLargeEnd = strchr(firstLargeStart+1, '}');
	
	memcpy(freq, cc, firstLargeStart-cc-1);
	freq[firstLargeStart-cc-1]=0;
	
	// 翻寫新的架構	 0:monthly 1:weekly 2:daily 3:once
	if(strcmp(freq, "monthly") == 0)
		se->pu8ScheduleTime[0]=0;
	else if (strcmp(freq, "weekly") == 0)
		se->pu8ScheduleTime[0]=1;
	else if (strcmp(freq, "daily") == 0)
		se->pu8ScheduleTime[0]=2;
	else if (strcmp(freq, "once") == 0)
		se->pu8ScheduleTime[0]=3;			
	
	memcpy(firstLarge, firstLargeStart+1, firstLargeEnd-firstLargeStart-1);
	firstLarge[firstLargeEnd-firstLargeStart-1]=0;
	
	tmp = strchr(firstLarge, ' ');
	memcpy(tmpstr1, firstLarge, tmp-firstLarge);
	tmpstr1[tmp-firstLarge]=0;
	
	//西元年
	if(tmpstr1[0]=='*')
		se->pu8ScheduleTime[1]=0xff;
	else
	{
		se->pu8ScheduleTime[1]=atoi(tmpstr1);
		//printf("Year ==> %s [%d]  \n",tmpstr1,se->pu8ScheduleTime[1] );
  }
	prevtmp = tmp+1;
	tmp = strchr(prevtmp, ' ');
	memcpy(tmpstr2, prevtmp, tmp-prevtmp);
	tmpstr2[tmp-prevtmp]=0;
	
	// 月份
	if(tmpstr2[0]=='*')
		se->pu8ScheduleTime[2]=0xff;
	else
		se->pu8ScheduleTime[2]=atoi(tmpstr2);

	prevtmp = tmp+1;
	tmp = strchr(prevtmp, ' ');
	memcpy(tmpstr3, prevtmp, tmp-prevtmp);
	tmpstr3[tmp-prevtmp]=0;
	// 日期
	if(tmpstr3[0]=='*')
		se->pu8ScheduleTime[3]=0xff;
	else
		se->pu8ScheduleTime[3]=atoi(tmpstr3);

	prevtmp = tmp+1;
	tmp = strchr(prevtmp, ' ');
	memcpy(tmpstr4, prevtmp, tmp-prevtmp);
	tmpstr4[tmp-prevtmp]=0;
	// 星期別
	if(tmpstr4[0]=='*')
		se->pu8ScheduleTime[4]=0xff;
	else
		se->pu8ScheduleTime[4]=atoi(tmpstr4);

	prevtmp = tmp+1;
	tmp = strchr(prevtmp, ' ');
	memcpy(tmpstr5, prevtmp, tmp-prevtmp);
	tmpstr5[tmp-prevtmp]=0;
	
	// 小時
	if(tmpstr5[0]=='*')
		se->pu8ScheduleTime[5]=0xff;
	else
		se->pu8ScheduleTime[5]=atoi(tmpstr5);

	prevtmp = tmp+1;
	strcpy(tmpstr6, prevtmp);
	tmpstr6[tmp-prevtmp]=0;
	// 分鐘
	if(tmpstr6[0]=='*')
		se->pu8ScheduleTime[6]=0xff;
	else
		se->pu8ScheduleTime[6]=atoi(tmpstr6);
	

	secondLargeStart = strchr(firstLargeEnd+1, '{');
	secondLargeEnd = strchr(secondLargeStart+1, '}');
	memcpy(secondLarge, secondLargeStart+3, secondLargeEnd-secondLargeStart-3);
	secondLarge[secondLargeEnd-secondLargeStart-3]=0;
	
	//tagobject data	
	prevtmp = secondLarge;
	tmp = strchr(prevtmp, ' ');
	memcpy(tmpstr7, prevtmp, tmp-prevtmp);
	tmpstr7[tmp-prevtmp]=0;

	se->cmnd.u32deviceid = atoi(tmpstr7);
  
	//command information
	prevtmp = tmp+1;
	tmp = strchr(prevtmp, ' ');
	
	se->cmnd.pu8ParaBuf = (U8*)malloc(sizeof(U8)*20);//固定初始化20 byte 10 word
	
	if(tmp == NULL)
	{ //indicate no para
		strcpy(tmpstr8, prevtmp);
		tmpstr9[0]=0;
		//se->cmnd.pTag = TagSystemAdaptor::FindTag(se->cmnd.u32deviceid, tmpstr8);
		strcpy(se->cmnd.name, tmpstr8);//參考command.h 指到command的名字
		//printf("action->%s \n",se->cmnd.name);
	}
	else
	{ //with para
		memcpy(tmpstr8, prevtmp, tmp-prevtmp);
		tmpstr8[tmp-prevtmp]=0;
		//se->cmnd.pTag = TagSystemAdaptor::FindTag(se->cmnd.u32deviceid, tmpstr8);
		strcpy(se->cmnd.name, tmpstr8);//參考command.h 指到command的名字

		strcpy(tmpstr9, tmp+1);
		
		//只實作一個參數
		sscanf(tmpstr9,"%f",(float*)se->cmnd.pu8ParaBuf);//printf("(%s) %f\n",tmpstr9,*(float*)se->cmnd.pu8ParaBuf);
	}
}



// 初始排程
int SchedulingSystem::InitScheduling(char *filename)
{ 
	//依設定檔讀取檔案
	char buffer[255];	  
	
	system_state=0;
	
	sprintf(buffer,"%s",filename);
	int FileLen=0;

	FILE *f = fopen(buffer, "r");
	if(f==0)
	{
		printf("fail open schedule file:%s\n",filename);
		return 0;//fail
	}
	
	char fileline[1024];
  char *filelineidx, c;
  int i=0, j=0, cmndid=0;
  
  //printf("InitScheduling \n");

	MAX_SchedulingObject = util::CalculateNewLineInFile(f);
	printf("Sread: %d line\n",MAX_SchedulingObject);
	fseek(f, 0, SEEK_SET);
	pSeObj = (SchedulingObject*)malloc(sizeof(SchedulingObject) * MAX_SchedulingObject);
	memset(pSeObj, 0, sizeof(SchedulingObject) * MAX_SchedulingObject);
	
  while(fgets(fileline,1024,f) != NULL)
  {

  	int devid;
  	fileline[strlen(fileline)-2]=0;
  	
  	//判斷是否不是 # ,98/10/17
  	if (isalnum(fileline[0]))
  	{

  		ParsingSchedulingConfig(fileline, &pSeObj[i]);
  		pSeObj[i].u8TImecheckInterval = 3;//1 min
			
//			printf("Scheduling (%d):%d, {%d %d %d %d %d %d %d},para:%s\n",i, pSeObj[i].deviceid, pSeObj[i].pu8ScheduleTime[0],pSeObj[i].pu8ScheduleTime[1],pSeObj[i].pu8ScheduleTime[2],pSeObj[i].pu8ScheduleTime[3],pSeObj[i].pu8ScheduleTime[4],pSeObj[i].pu8ScheduleTime[5],pSeObj[i].pu8ScheduleTime[6],pSeObj[i].para);	 		 			  
			/*
			printf("No [%d]  DateTime ==> %d %d %d %d %d %d %d\n",i, pSeObj[i].pu8ScheduleTime[0],
			pSeObj[i].pu8ScheduleTime[1],pSeObj[i].pu8ScheduleTime[2],pSeObj[i].pu8ScheduleTime[3],
			pSeObj[i].pu8ScheduleTime[4],pSeObj[i].pu8ScheduleTime[5] ,
		 	pSeObj[i].pu8ScheduleTime[6]);	  
			*/
			//printf(" para  %s \n",pSeObj[i].para);
		  i++;
		}
  }
  fclose(f);	
  system_state=1;
  return 1;
}


// 釋放  Scheduling 
void SchedulingSystem::FreeScheduling()
{
	for(int i=0; i<MAX_SchedulingObject; i++)
	{
		free(pSeObj[i].cmnd.pu8ParaBuf);
	}
	free(pSeObj);
}


int TM_CheckNull(struct tm *_time)
{
	if(
	_time->tm_sec  == 0 &&
	_time->tm_min  == 0 &&
	_time->tm_hour == 0 &&
	_time->tm_mday == 0 &&
	_time->tm_mon  == 0 &&
	_time->tm_year == 0 &&
	_time->tm_wday == 0 &&
	_time->tm_yday == 0 &&
	_time->tm_isdst== 0)
	
	return 1;//it's null time object
	
	return 0;//it's not a null time object
}


void OSDepend_gettimeofday(iENtv *tv)
{
	gettimeofday(&(tv->_val), NULL);
}

int OSDepend_GetTimeValueSec(iENtv *tv)
{
	return tv->_val.tv_sec;
}


//  重新 Reload Scheduling 
int SchedulingSystem::ReloadScheduling(char *fileName)
{	
	//suspend shceduling 
	while(system_state != 1)
	{
		usleep(10000);
	}
	FILE * fnew, *fnewout;
	int c;
	
	fnew = fopen(fileName, "r");
	if(fnew == 0)
	{
		system_state=1;
		return 0;
	}
	
	system_state=0;
	FreeScheduling();
	rename( "schedule.txt" , "schedule.bak");
	fnewout = fopen("./schedule.txt", "w");
	do {
      c = fgetc (fnew);
      fputc((char)c, fnewout);
  } while (c != EOF);
	fclose(fnew);
	fclose(fnewout);
	
	if(InitScheduling("./schedule.txt")==1)
  	return 1;
  else
  {//fail roll back to the orginal file.
  	rename( "schedule.bak" , "schedule.txt");
  	//suspend shceduling 
		while(system_state != 1)
		{
			usleep(10000);
		}
		
		system_state=0;
		FreeScheduling();
		InitScheduling("./schedule.txt");
  	return 0;
  }
}

void *ReloadSchedulingController(char *filename)
{
	((SchedulingSystem*)module)->ReloadScheduling(filename);
	return 0;
}
	
extern "C"
{
	void *reload(void *para)
	{
		char *filename = (char*)para;
		ReloadSchedulingController(filename);
		return 0;
	}
}
