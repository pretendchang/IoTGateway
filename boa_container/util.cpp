
#include "util.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


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
//字串中無特殊符號 regexp實作
void util::splitstring(char *in, char *splitter, int fieldcount, char ***out)
{
	char ptn[500];
	regex_t a;
	char *iterator = in;
	int status;
	regmatch_t *pm;
	char ptnbase[40];
	
	if(splitter[0]=='.')
		strcpy(ptnbase, "([_0-9a-Z]*)");
	else
		strcpy(ptnbase, "([\\._0-9a-Z]*)");
	
	pm = (regmatch_t*)malloc(sizeof(regmatch_t) * (fieldcount+1));
	
	int i;
	memset(ptn, 0, 500);
	for(i=0; i<fieldcount-1; i++)
	{
		strcat(ptn, ptnbase);
		strcat(ptn, splitter);
	}
	strcat(ptn, ptnbase);
	
	regcomp(&a, ptn, REG_EXTENDED|REG_ICASE);
	
	status = regexec(&a, iterator, (size_t)(fieldcount+1), pm, 0);

	if(status==1)
	{
	 	printf("fail regexec\n");
	 	return;
	}
	for(i=1; i<fieldcount+1;i++)
	{	
	 	memcpy((*out)[i-1], iterator + pm[i].rm_so, pm[i].rm_eo-pm[i].rm_so);
	 	(*out)[i-1][pm[i].rm_eo-pm[i].rm_so]=0;
	 	
//	 	printf("Get(%d):%s\n",i,(*out)[i-1]);
	}

  regfree(&a);
  free(pm);
}

//字串中有特殊符號 strchr實作
void util::splitstring1(char *in, char *splitter, int fieldcount, char ***out)
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

void util::formatValue(char **ptn_str, void *value, char *out)
{	//only support d f specifier
	char chrfmt[10];
	char *ptn=*ptn_str;
	int j=0;
	while((*ptn)!=0)
	{
		if((*ptn) == 'd' || (*ptn) == 'f')
		{
			chrfmt[j++]=(*ptn);
			chrfmt[j]=0;
			switch((*ptn))
			{
			case 'd':
				sprintf(out,chrfmt,*(int*)value);
				break;
		
			case 'f':
				sprintf(out,chrfmt,*(float*)value);
				
				break;
			}
			ptn++;
			break;
		}
		else
		{
			chrfmt[j++]=(*ptn);
		}
		ptn++;
	}
	*ptn_str=ptn;
}

void util::format(char *out, char *ptn, arraylist *v)//the element in v is 32 bit
{
	int iVectorCnt=0;
	char one_c[2];
	char *ptn_c=ptn;
	for(int i=0; i<strlen(ptn); i++)
	while((*ptn_c) !=0 )
	{
		if((*ptn_c) != '%')
		{
			one_c[0]=*ptn_c;
			one_c[1]=0;
			strcat(out, one_c);
			ptn_c++;
		}
		else
		{
			char c[10];
			
			formatValue(&ptn_c,v->pop(iVectorCnt++), c);
			strcat(out, c);
		}
	}
}

void util::replace(char *str, char from, char to)
{
	char *it, *previt;
	previt = str;

	while((it=strchr(previt, from))!=NULL)
	{
		*it=to;
		previt=it+1;
	}
}

#include <pthread.h>
#include <sys/time.h>
void util::timestamplog(char *msg)
{
		char *strtime, pOut[256];
		time_t rawtime;
		struct timeval tv;
	
	  FILE *f=fopen("./timestamplog.txt","a");
	
		time(&rawtime);
		strtime = ctime(&rawtime);
		strtime[strlen(strtime)-1]=0;
		gettimeofday(&tv, NULL);
		sprintf(pOut, "%s,%d,", strtime,tv.tv_sec*1000+tv.tv_usec/1000);
		
		fwrite(pOut, 1, strlen(pOut), f);
		fwrite(msg, 1, strlen(msg), f);
		
		fwrite("\n",1,1,f);
		
		fclose(f);
}
