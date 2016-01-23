#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "custom1.h"

void InitModuleObject()
{
	module = new Custom1System();
}

void Custom1System::Init()
{
	cmndr.u32deviceid=100;
	strcpy(cmndr.name, "pow11");
	cmndr.pu8ParaBuf=(U8*)malloc(4);
	
	cmndw.u32deviceid=101;
	strcpy(cmndw.name, "show");
	cmndw.pu8ParaBuf=(U8*)malloc(2);
	
	cmndd.u32deviceid=101;
	strcpy(cmndd.name, "dow");
	cmndd.pu8ParaBuf=(U8*)malloc(2);
	baseYear=0;
	baseMonth=0;
	baseDay=0;
	
}

void Custom1System::DispatchEvent()
{
	U8 u8State=0;
	
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	GetValue(cmndr.u32deviceid, cmndr.name, cmndr.pu8ParaBuf,&u8State);
	if(baseYear!=p->tm_year || baseMonth!=p->tm_mon || baseDay!=p->tm_mday)
	{
		UpdateTime();
		memcpy(&baseValue, cmndr.pu8ParaBuf, sizeof(float));
		
		float *pw=(float*)cmndw.pu8ParaBuf;
		float *pi=(float*)cmndd.pu8ParaBuf;
		*pw=0;
		*pi=4;
		UpdateTime();
		
	}
	else
	{	
		float *pw=(float*)cmndw.pu8ParaBuf;
		float *pr=(float*)cmndr.pu8ParaBuf;
		float *pi=(float*)cmndd.pu8ParaBuf;
		int i2f;
		float f=(*pr)-baseValue;
		i2f=(int)f;
		if(i2f>=1000)
		{	
			(*pw)=(float)(i2f%10000);
			*pi=4;
		}
		else if(i2f>=100)
		{	
			(*pw)=(float)(f*10);
			*pi=3;
		}
		else if(i2f>=10)
		{	
			(*pw)=(float)(f*100);
			*pi=2;
		}
		else
		{	
			(*pw)=(float)(f*1000);
			*pi=1;
		}
	}
	SetValue(cmndw.u32deviceid, cmndw.name, cmndw.pu8ParaBuf);
	SetValue(cmndd.u32deviceid, cmndd.name, cmndd.pu8ParaBuf);
}

void Custom1System::UpdateTime()
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	baseYear=p->tm_year;
  baseMonth=p->tm_mon;
  baseDay=p->tm_mday;
}


