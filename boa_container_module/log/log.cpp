#include "common.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tagsystemAdaptor.h"

void InitModuleObject()
{
	printf("init here\n");
	module = new LogSystem();
}

U8 LogEvent::GetLogValue()
{
	U8 u8State;
	TagSystemAdaptor::ExecuteLog(&cmnd,&u8State);
	return u8State; //success
}

int LogEvent::CheckLogTiming()
{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		//printf("logtiming:%d %d\n",this->LastTimeStamp,tv.tv_sec);
		if((this->LastTimeStamp + this->u32Interval)<=tv.tv_sec)
		{
			this->LastTimeStamp = tv.tv_sec;
			return 1;//log執行時間已到
		}
		return 0;
}

void LogSystem::UpdateSystime()
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	timenow.time_1y=p->tm_year+1900;
	timenow.time_2y=p->tm_year-11;
  timenow.time_M=p->tm_mon+1;
  timenow.time_d=p->tm_mday;
  timenow.time_w=p->tm_wday+1;
  timenow.time_1h=p->tm_hour;
  timenow.time_m=p->tm_min;
  timenow.time_s=p->tm_sec;
}

void LogSystem::DispatchLogEvent()
{
	int i=0;//printf("DispatchLogEvent\n");
	char strTimeStamp[50];
	char strDevid[20];
	int isNewFile=0;
	
	UpdateSystime();
	
	for(i=0; i<MAX_LogEvent; i++)
	{
		FILE *logfile;
		if(!LogEvt[i].CheckLogTiming())
			continue;//時間還沒到
			
		//open file
		char slogfilename[100], slogcontent[10000];
		slogfilename[0]=0;slogcontent[0]=0;
		util::format(slogfilename, LogEvt[i].clogFileNameFmtString, &LogEvt[i].plogFileNameFmtPara);
		
		char fullpath[200];
		sprintf(fullpath,"%s.txt",slogfilename);
		logfile = fopen(fullpath, "r");
		if(logfile==0)
		{
			isNewFile=1;//it's a new file
		}
		else
			fclose(logfile);
		logfile = fopen(fullpath,"a");

		if(LogEvt[i].GetLogValue() == 1)
		{
			util::format(slogcontent, LogEvt[i].cFieldFmtString, &LogEvt[i].pFieldFmtPara);
			if(isNewFile)
			{//it's a new file, print out the column name at first
				fwrite(LogEvt[i].cFieldNameString, 1, strlen(LogEvt[i].cFieldNameString), logfile);
				fwrite("\n", 1, 1, logfile);
			}
			fwrite(slogcontent, 1, strlen(slogcontent), logfile);
			fwrite("\n", 1, 1, logfile);
		}
		else
		{
			if(isNewFile)
			{//it's a new file, print out the column name at first
				fwrite(LogEvt[i].cFieldNameString, 1, strlen(LogEvt[i].cFieldNameString), logfile);
				fwrite("\n", 1, 1, logfile);
			}
			fwrite("fail communication\n",1,19,logfile);
		}
		fclose(logfile);
	}
}

void LogSystem::InitPara(TagObject *obj, char *para, arraylist *paraValue)
{
	char **out;
	int i, cnt=0;
	char *it = para;
	while((it=strchr(it, '|'))!=NULL)
	{
		it++;
		cnt++;
	}
	cnt++;
	out=(char**)malloc(4 * cnt);
	for(i=0;i<cnt;i++)
	{
		out[i]=(char*)malloc(50);
	}
	util::splitstring1(para, "|", cnt, &out);

	for(i=0;i<cnt;i++)
	{
		if(strcmp(out[i],"@time_1y") == 0)
		{
			paraValue->push(&timenow.time_1y);
		}
		else if(strcmp(out[i],"@time_2y") == 0)
		{
			paraValue->push(&timenow.time_2y);
		}
		else if(strcmp(out[i],"@time_M") == 0)
		{
			paraValue->push(&timenow.time_M);
		}
		else if(strcmp(out[i],"@time_d") == 0)
		{
			paraValue->push(&timenow.time_d);
		}
		else if(strcmp(out[i],"@time_w") == 0)
		{
			paraValue->push(&timenow.time_w);
		}
		else if(strcmp(out[i],"@time_1h") == 0)
		{
			paraValue->push(&timenow.time_1h);
		}
		else if(strcmp(out[i],"@time_m") == 0)
		{
			paraValue->push(&timenow.time_m);
		}
		else if(strcmp(out[i],"@time_s") == 0)
		{
			paraValue->push(&timenow.time_s);
		}
		else if(strcmp(out[i],"@devid") == 0)
		{
			paraValue->push(&obj->u32deviceid);
		}
		else
		{
			char *idx = out[i]+1;
			paraValue->push(obj->pu8ParaBuf+((atoi(idx)-1)*sizeof(ParameterType)));
		}
	}	
		
		
	for(i=0;i<cnt;i++)
	{
		free(out[i]);
	}
	free(out);
}

void LogSystem::InitLog(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open log file:%s\n",filename);
		
	char fileline[2048];

	char **out;
	int i, j=0;
	out=(char**)malloc(4 * LogSystem::fieldcount);
	for(i=0;i<LogSystem::fieldcount;i++)
	{
		out[i]=(char*)malloc(2048);
	}
  
  MAX_LogEvent = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	LogEvt = (LogEvent*)malloc(sizeof(LogEvent) * MAX_LogEvent);
	memset(LogEvt, 0, sizeof(LogEvent) * MAX_LogEvent);

  while(fgets(fileline,2048,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring1(fileline, ",", LogSystem::fieldcount, &out);LogEvt[j].u32Interval =0;

		LogEvt[j].cmnd.u32deviceid=atoi(out[2]);
		//LogEvt[j].cmnd.pTag = TagSystemAdaptor::FindTag(LogEvt[j].cmnd.u32deviceid, out[3]);
		strcpy(LogEvt[j].cmnd.name, out[3]);//參考command.h 指到command的名字
		LogEvt[j].cmnd.pu8ParaBuf=(U8*)malloc(atoi(out[7]) * sizeof(ParameterType));

 		LogEvt[j].u32Interval = atoi(out[4]);
 		
 		util::replace(out[0], '|', ',');
	  strcpy(LogEvt[j].clogFileNameFmtString, out[0]);
	  InitPara(&LogEvt[j].cmnd, out[1], &LogEvt[j].plogFileNameFmtPara);
		
		util::replace(out[5], '|', ',');
		strcpy(LogEvt[j].cFieldFmtString,out[5]);
		InitPara(&LogEvt[j].cmnd, out[6], &LogEvt[j].pFieldFmtPara);
		
		util::replace(out[8], '|', ',');
		strcpy(LogEvt[j].cFieldNameString,out[8]);
		
		j++;		
  }
  
  for(i=0;i<LogSystem::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}

void LogSystem::FreeLog()
{
	int i;
	free(LogEvt);
}

