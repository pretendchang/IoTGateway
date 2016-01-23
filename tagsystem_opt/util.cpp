
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int util::CalculateNewLineInFile(FILE *f)
{//最後要多留空白一行
	char c;
	int newline=0;
	if(f == NULL)
		return -1;
	
	while((c = fgetc(f)) != 0xff)
	{//printf("%x\n",(int)c);
		if(c == '\n')
			newline++;	
	}	
	return newline;
}

void util::splitstring(char *in, char *splitter, int fieldcount, char ***out)
{
	char *it, *previt;
	int i=0;
	previt = in;

	while((it=strchr(previt, *splitter))!=NULL)
	{
		memcpy((*out)[i++], previt, it-previt);
		(*out)[i-1][it-previt]=0;
		previt=it+1;
	}
	strcpy((*out)[i++], previt);
}

U32 util::exp1(U32 base, U32 time)
{
	U32 i=0;
	U32 ret=1;
	for(i=0; i<time; i++)
	{
		ret*=base;
	}
	return ret;
}

U32 util::BCD2Dec(U32 bcd)
{
	U32 tmp = bcd;
	U32 ret=0, i =0;
	while(tmp != 0)
	{
		ret += (tmp & 0xf)*exp1(10,i);
		i++;
		tmp = (tmp>>(i*4));
	}
	return ret;
}

U32 util::Dec2BCD(U32 dec)
{
	U32 tmp=dec;
	U32 i=0, ret=0;;
	while(tmp!=0)
	{
		ret += (tmp%exp1(10,i+1))*exp1(16,i);
		tmp=tmp/10;
		i++;
	}
	return ret;
}

float util::atoff(char *str)
{
	float ret=0;
	char strInt[10], strPart[10], *dot;
	if((dot=strchr(str, '.')) !=0)
	{
		memcpy(strInt, str, dot-str);
		strInt[dot-str]=0;
		strcpy(strPart, dot+1);
		switch(strlen(strPart))
		{
			case 0:
				break;
			case 1:
				ret += atoi(strPart)/10.0;
				break;
			case 2:
				ret += atoi(strPart)/100.0;
				break;
			case 3:
				ret += atoi(strPart)/1000.0;
				break;
		}
		ret += (float)atoi(strInt);
	}
	else
		ret = (float)atoi(str);
		
	return ret;
}

void util::DegPrint(char *fmt, ...)
{
#ifdef _DbgPrint_	
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	printf("%s",buffer);
	va_end(args);
#endif	
}

void util::timestamplog(char *msg)
{
	/*
		char *strtime, pOut[256];
		time_t rawtime;
		struct timeval tv;
	
	  FILE *f=fopen("./timestamplog.txt","a");
	
		time(&rawtime);
		strtime = ctime(&rawtime);
		strtime[strlen(strtime)-1]=0;
		gettimeofday(&tv, NULL);
		sprintf(pOut, "%s, %d, %d,", strtime,tv.tv_sec,tv.tv_usec);
		
		fwrite(pOut, 1, strlen(pOut), f);
		fwrite(msg, 1, strlen(msg), f);
		
		fwrite("\n",1,1,f);
		
		fclose(f);*/
}
