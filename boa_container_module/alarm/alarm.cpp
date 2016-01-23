

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

#include <ctype.h>    // toascii用

#include "alarm.h"
#include "tagsystemAdaptor.h"

void InitModuleObject()
{
	module = new AlarmSystem();
}


// 將oid --> len X 的   OID
void FireOID::OIDStr2OID(char *oidstr, U8 *oid, U8 *oidlen)
{
	int data = atoi(oidstr);
	int n = 128, i=0, j;
	U8 significant = 0;
	int multiple = 1;
	U8 u8OIDElm[10];

	while (data != 0)
  {
		int temp = data % n;
		data -= temp;

		temp = temp / multiple;
		temp += significant;
		u8OIDElm[i] = temp;

		significant = 128;
		multiple *= 128;
		n *= n;
		i++;
  }
  
	*oidlen = i;
	for(j=0; j<*oidlen; j++)
	{
		oid[j] = u8OIDElm[*oidlen-j-1];
		
	}
}

//解碼
void FireOID::DecodeOID(char *oidstr, U8 *oid, U8 *oidlen)
{
		char *tmp, *prevtmp;
		char oidelm[20];
		U8 u8OIDElm[10], u8OIDElmLen;

		prevtmp = oidstr;
		*oidlen = 0;
		int i=0;
		while((tmp = strchr(prevtmp ,'.')) != NULL)
		{
			memcpy(oidelm, prevtmp, tmp-prevtmp);
			oidelm[tmp-prevtmp]=0;
			OIDStr2OID(oidelm, u8OIDElm, &u8OIDElmLen);
			//now I get the element value, copy it to TxBuf
			for(i=0; i<u8OIDElmLen; i++)
			{
				oid[*oidlen] = u8OIDElm[i];
				(*oidlen)++;
			}

			prevtmp = tmp+1;
		}
		strcpy(oidelm, prevtmp);
		OIDStr2OID(oidelm, u8OIDElm, &u8OIDElmLen);
		//now I get the element value, copy it to TxBuf
		for(i=0; i<u8OIDElmLen; i++)
		{
				oid[*oidlen] = u8OIDElm[i];
				(*oidlen)++;
		}
		oid[0]=(oid[0]*40+oid[1])&0xff;
		(*oidlen)--;
		for(i=1;i<(*oidlen);i++)
		{
			oid[i]=oid[i+1];
		}
}



//發送 SnmpTrap
void FireOID::SendSNMPTrap(int *sockfd, char *oid, char *msg)
{//oid有特定的編碼規則  需釐清
	U8 u8TxBuf[1024];
  
	u8TxBuf[0]=0x30;
	//header
	u8TxBuf[2] = 0x02; u8TxBuf[3] = 0x01; u8TxBuf[4] = 0x00; u8TxBuf[5] = 0x04; u8TxBuf[6] = 0x06;
	u8TxBuf[7] = 0x70; u8TxBuf[8] = 0x75; u8TxBuf[9] = 0x62; u8TxBuf[10] = 0x6C; u8TxBuf[11] = 0x69; u8TxBuf[12] = 0x63;

	u8TxBuf[13]=0xa4; u8TxBuf[14]=0;//trap pdu header

	//trap pdu
	//source ID
	u8TxBuf[15]=0x6; u8TxBuf[16]=0x8;
	//private Ent
	u8TxBuf[17]=0x2b;u8TxBuf[18]=0x6;u8TxBuf[19]=0x1;u8TxBuf[20]=0x4;
	u8TxBuf[21]=0x1;
	u8TxBuf[22]=0x81;
	u8TxBuf[23]=0x83;
	u8TxBuf[24]=0x64;

	//source IP
	u8TxBuf[25]=0x40; u8TxBuf[26]=0x4; 
	u8TxBuf[27]=10;u8TxBuf[28]=77;u8TxBuf[29]=13;u8TxBuf[30]=22;//sender ip todo

	//Generic status
	u8TxBuf[31]=2; u8TxBuf[32]=1; u8TxBuf[33]=6;//input

	//trap status specific
	u8TxBuf[34]=2; u8TxBuf[35]=1; u8TxBuf[36]=0;

	//time since last trap
	u8TxBuf[37]=0x43; u8TxBuf[38]=1; u8TxBuf[39]=0;

	//msg
	u8TxBuf[40]=0x30; u8TxBuf[41]=0;
	u8TxBuf[42]=0x30; u8TxBuf[43]=0;

	//OID
	u8TxBuf[44]=0x6;	u8TxBuf[45]=0;
	DecodeOID(oid, u8TxBuf+46, u8TxBuf+45);//decode OID

	//Message
	u8TxBuf[45+u8TxBuf[45]+1]=0x4;u8TxBuf[45+u8TxBuf[45]+2]=strlen(msg);
	memcpy(&u8TxBuf[45+u8TxBuf[45]+3], msg, strlen(msg));

	u8TxBuf[41]= 45+u8TxBuf[45]+3+strlen(msg)-42;
	u8TxBuf[43]= 45+u8TxBuf[45]+3+strlen(msg)-44;
	u8TxBuf[14]= 45+u8TxBuf[45]+3+strlen(msg)-15;
	u8TxBuf[1] = 45+u8TxBuf[45]+3+strlen(msg)-2;
	
	
	printf("發 送 Snmp \n");
	//發 送 snmp
	send(*sockfd, u8TxBuf, u8TxBuf[1]+2, 0);
	
	printf("發 送 Snmp  End\n");
}

//初始 Init UDP Commu
void FireOID::InitUDPCommu(int *sockfd, char *strip)
{	
	struct sockaddr_in dest;
	char buffer[128];
	char rbuffer[128];

	int flag;
	*sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	bzero(&dest, sizeof(dest));
	dest.sin_family=PF_INET;
	dest.sin_port=htons(162);

	inet_aton(strip, &dest.sin_addr);
	
	connect(*sockfd, (struct sockaddr*)&dest, sizeof(dest));
}

// Close UDP commu
void FireOID::CloseUDPCommu(int *sockfd)
{
	close(*sockfd);
}

//組合　Context文字
void AlarmSystem::ParseAlarmContent(AlarmContent *palmcnt, char *alarmcnt)
{
}


// Reload  Alarm
int AlarmSystem::ReloadAlarmContent(char *fileName)
{
  return 1;
}

/*  初始化 Init Alarm 
 Init Alarm 
 */
void AlarmSystem::InitAlarmContent()
{

}

void AlarmContent::Threshold2Rule(U8 idx, char *mainThreshold, char *concern, char *pairThreshold, Rule *r)
{
	Condition *mainCondition, *tmpMainCond, *tmpMainCond1;
	char BoundThreshold1[10], BoundThreshold2[10];
//	r->prev=0;
	
	ParseThreshold(mainThreshold, BoundThreshold1, BoundThreshold2);
	
	switch(idx)
	{
		case 1://
			r->cond.rvalue.value=(void*)&r->cond.rvalue.u32Constant;
			r->cond.rvalue.u32Constant=atoi(BoundThreshold1);
			r->cond.rvalue.valuetype=ConditionValueType_INT;
			r->cond.relation=ConditionRelation_SMALLER_EQUAL;
			r->cond.lvalue.valuetype=ConditionValueType_FLOAT;
			r->cond.lvalue.value=(void*)r->cmnd[0].pu8ParaBuf;
			
			mainCondition = &r->cond;
			break;
			
		case 2://
			r->cond.rvalue.value=(void*)&r->cond.rvalue.u32Constant;
			r->cond.rvalue.u32Constant=atoi(BoundThreshold1);
			r->cond.rvalue.valuetype=ConditionValueType_INT;
			r->cond.relation=ConditionRelation_GREATER_EQUAL;
			r->cond.lvalue.valuetype=ConditionValueType_FLOAT;
			r->cond.lvalue.value=(void*)r->cmnd[0].pu8ParaBuf;		
			
			tmpMainCond = new Condition();
			r->cond.next = tmpMainCond;
			tmpMainCond->rvalue.value=(void*)&tmpMainCond->rvalue.u32Constant;
			tmpMainCond->rvalue.u32Constant=atoi(BoundThreshold2);
			tmpMainCond->rvalue.valuetype=ConditionValueType_INT;
			tmpMainCond->relation=ConditionRelation_SMALLER_EQUAL;
			tmpMainCond->lvalue.valuetype=ConditionValueType_FLOAT;
			tmpMainCond->lvalue.value=(void*)r->cmnd[0].pu8ParaBuf;
						
			tmpMainCond1 = new Condition();
			tmpMainCond->next=tmpMainCond1;
			tmpMainCond1->rvalue.value=(void*)&r->cond.ret;
			//tmpMainCond1->rvalue.u32Constant=atoi(BoundThreshold2);
			tmpMainCond1->rvalue.valuetype=ConditionValueType_CONDITION;
			tmpMainCond1->relation=ConditionRelation_AND;
			tmpMainCond1->lvalue.valuetype=ConditionValueType_CONDITION;
			tmpMainCond1->lvalue.value=(void*)&tmpMainCond->ret;
			
			mainCondition = tmpMainCond1;
			break;	
			
		case 3:	
			r->cond.rvalue.value=(void*)&r->cond.rvalue.u32Constant;
			r->cond.rvalue.u32Constant=atoi(BoundThreshold1);
			r->cond.rvalue.valuetype=ConditionValueType_INT;
			r->cond.relation=ConditionRelation_GREATER_EQUAL;
			r->cond.lvalue.valuetype=ConditionValueType_FLOAT;
			r->cond.lvalue.value=(void*)r->cmnd[0].pu8ParaBuf;
			
			mainCondition = &r->cond;
			break;
	}
	
	
	
	
	if(strcmp(concern, "1") == 0)
	{
		Condition *cond = new Condition();
		mainCondition->next=cond;
		cond->prev = mainCondition;
		//pairThreshold1 <= const
		cond->lvalue.valuetype=ConditionValueType_FLOAT;
		cond->lvalue.value=(void*)r->cmnd[1].pu8ParaBuf;
		
		cond->rvalue.value=(void*)&cond->rvalue.u32Constant;
		ParseThreshold(pairThreshold, BoundThreshold1, BoundThreshold2);
		cond->rvalue.u32Constant=atoi(BoundThreshold2);
		cond->rvalue.valuetype=ConditionValueType_INT;
		
		cond->relation=ConditionRelation_SMALLER_EQUAL;
		
		//pairThreshold1 >= const
		Condition *cond1 = new Condition();
		cond->next = cond1;
		cond1->prev=cond;

		cond1->lvalue.valuetype=ConditionValueType_FLOAT;
		cond1->lvalue.value=(void*)r->cmnd[1].pu8ParaBuf;
		
		cond1->rvalue.value=(void*)&cond1->rvalue.u32Constant;
		cond1->rvalue.u32Constant=atoi(BoundThreshold1);
		cond1->rvalue.valuetype=ConditionValueType_INT;
		
		cond1->relation=ConditionRelation_GREATER_EQUAL;
		
		//pairThreshold1 < const && pairThreshold1 > const
		Condition *cond2 = new Condition();
		cond1->next = cond2;
		cond2->prev=cond1;
		
		cond2->lvalue.valuetype=ConditionValueType_CONDITION;
		cond2->lvalue.value=(void*)&(cond->ret);
		
		cond2->rvalue.value=(void*)&(cond1->ret);
		cond2->rvalue.valuetype=ConditionValueType_CONDITION;
		
		cond2->relation=ConditionRelation_AND;
		
		//mainThreshold1 && (pairThreshold1 < const && pairThreshold1 > const)
		Condition *cond3 = new Condition();
		cond3->next=0;
		cond2->next = cond3;
		cond3->prev=cond2;

		cond3->lvalue.valuetype=ConditionValueType_CONDITION;
		cond3->lvalue.value=(void*)&(mainCondition->ret);
		
		cond3->rvalue.value=(void*)&(cond2->ret);
		cond3->rvalue.valuetype=ConditionValueType_CONDITION;
		
		cond3->relation=ConditionRelation_AND;
	}
	else
	{
		r->cond.next=0;
	}
}
void AlarmContent::Init(datasnmp data)
{
	sprintf(sPublicIp, "%s.%s.%s.%s", data.ip1,data.ip2,data.ip3,data.ip4);

	if(strcmp(data.ruletype, "2")==0)
	{
		char t1[10], t2[10];
		u32Freq = atoi(data.interval);
		u32Delay = atoi(data.delay);
		ParseTime(data.starttime, t1, t2);
		pu8StartTime[0]=2;pu8StartTime[5]=atoi(t1);pu8StartTime[6]=atoi(t2);
		
		ParseTime(data.stoptime, t1, t2);
		pu8StopTime[0]=2;pu8StopTime[5]=atoi(t1);pu8StopTime[6]=atoi(t2);
		datasnmp2Condition(data);
	}
	else if(strcmp(data.ruletype, "0")==0)
	{
		u32Freq = atoi(data.interval);
		u32Delay = atoi(data.delay);
		rule.ext = ((AlarmSystem*)module)->ruleextcol.FindRuleExt(data.name, 1);
	}
	else
	{
		rule.ext = ((AlarmSystem*)module)->ruleextcol.FindRuleExt(data.name, 1);
	}
}

void AlarmContent::ParseThreshold(char *sThreshold, char *out1, char *out2)
{//{?? ??}  ??
	if(out1 == NULL || out2 == NULL)
		return;//out should be malloc at first
		
	if(sThreshold[0] !='{')
	{
		strcpy(out1, sThreshold);
		out2[0]=0;
	}
	else
	{
		char *space;
		space = strchr(sThreshold, ' ');
		memcpy(out1, sThreshold+1, space- sThreshold);
		out1[space- sThreshold]=0;
		
		strcpy(out2, space+1);
		out2[strlen(out2)-1]=0;
	}
}

void AlarmContent::ParseValue(char *sValue, char *out1, char *out2)
{//@??::????
	if(out1 == NULL || out2 == NULL)
		return;//out should be malloc at first
		
	char *colon;
	colon = strchr(	sValue,':');
	
	memcpy(out1, sValue, colon-sValue);
	out1[colon-sValue]=0;
	colon+=2;
	strcpy(out2, colon);
}

void AlarmContent::ParseTime(char *sTime, char *out1, char *out2)
{//??:??
	if(out1 == NULL || out2 == NULL)
		return;//out should be malloc at first
		
	char *colon;
	colon = strchr(	sTime,':');
	
	memcpy(out1, sTime, colon-sTime);
	out1[colon-sTime]=0;
	colon+=1;
	strcpy(out2, colon);
}
	
void AlarmContent::datasnmp2Condition(datasnmp data)
{
	char c1[10],c2[20];
	Rule *r=0, *prevr=0, *r1=0, *r2=0;
	
	ParseValue(data.mainvalue,c1,c2);

	if(data.mainThreshold1[0]!=0)
	{
		rule.sOidIp=sPublicIp;	
		rule.cmnd[0].u32deviceid=atoi(c1+1);//based on mainValue
		
		//rule.cmnd[0].pTag = TagSystemAdaptor::FindTag(rule.cmnd[0].u32deviceid, c2);
		strcpy(rule.cmnd[0].name, c2);//參考command.h 指到command的名字
		rule.cmnd[0].pu8ParaBuf=(U8*)malloc(20);

		if(data.pairValue1[0] != 0)
		{
			ParseValue(data.pairValue1,c1,c2);
			
			rule.cmnd[1].u32deviceid=atoi(c1+1);//based on mainValue
			//rule.cmnd[1].pTag = TagSystemAdaptor::FindTag(rule.cmnd[1].u32deviceid, c2);
			strcpy(rule.cmnd[1].name, c2);//參考command.h 指到command的名字
		}
		rule.cmnd[1].pu8ParaBuf=(U8*)malloc(20);
		Threshold2Rule(1, data.mainThreshold1, data.concern1,data.pairThreshold1, &rule);
		strcpy(rule.msg, data.msg1);
		
		rule.ext = ((AlarmSystem*)module)->ruleextcol.FindRuleExt(data.name, 1);
		r = &rule;
	}

	if(data.mainThreshold2[0] != 0)
	{
		if(r == 0)
			r = &rule;
		else
		{
			r1=new Rule();
			prevr = r;
			r= r1;
			prevr->next=r;
			r->prev=prevr;
		}
		r->sOidIp=sPublicIp;
		r->cmnd[0].u32deviceid=atoi(c1+1);//based on mainValue
		//r->cmnd[0].pTag = TagSystemAdaptor::FindTag(r->cmnd[0].u32deviceid, c2);
		strcpy(r->cmnd[0].name,c2);//參考command.h 指到command的名字
		r->cmnd[0].pu8ParaBuf=(U8*)malloc(20);
	
		if(data.pairValue2[0] != 0)
		{
			ParseValue(data.pairValue2,c1,c2);
			r->cmnd[1].u32deviceid=atoi(c1+1);//based on mainValue
			//r->cmnd[1].pTag = TagSystemAdaptor::FindTag(r->cmnd[1].u32deviceid, c2);
			strcpy(r->cmnd[1].name,c2);//參考command.h 指到command的名字
		}
		r->cmnd[1].pu8ParaBuf=(U8*)malloc(20);
		Threshold2Rule(2, data.mainThreshold2, data.concern2,data.pairThreshold2, r);
		strcpy(r->msg, data.msg2);
		r->ext = ((AlarmSystem*)module)->ruleextcol.FindRuleExt(data.name, 2);
	}

	if(data.mainThreshold3[0] != 0)
	{
		if(r == 0)
			r = &rule;
		else
		{
			r2=new Rule();
			prevr = r;
			r= r2;
			prevr->next=r;
			r->prev=prevr;
			r->next=0;
		}
		r->sOidIp=sPublicIp;
		r->cmnd[0].u32deviceid=atoi(c1+1);//based on mainValue
		//r->cmnd[0].pTag = TagSystemAdaptor::FindTag(r->cmnd[0].u32deviceid, c2);
		strcpy(r->cmnd[0].name,c2);//參考command.h 指到command的名字
		r->cmnd[0].pu8ParaBuf=(U8*)malloc(20);
		
		if(data.pairValue3[0] != 0)
		{
			ParseValue(data.pairValue3,c1,c2);
			r->cmnd[1].u32deviceid=atoi(c1+1);//based on mainValue
			//r->cmnd[1].pTag = TagSystemAdaptor::FindTag(r->cmnd[1].u32deviceid, c2);
			strcpy(r->cmnd[1].name,c2);//參考command.h 指到command的名字
		}
		r->cmnd[1].pu8ParaBuf=(U8*)malloc(20);
		Threshold2Rule(3, data.mainThreshold3, data.concern3,data.pairThreshold3, r);
		strcpy(r->msg, data.msg3);
		r->ext = ((AlarmSystem*)module)->ruleextcol.FindRuleExt(data.name, 3);
	}
}

void AlarmSystem::getValue(char fileline, char *token, int *tokenidx, datasnmp *file, int *strstate)
{
	if(((*strstate)%3) == 2)
			{
				if(fileline>='!' && fileline<='~')
				{
					token[(*tokenidx)++]=fileline;
					(*strstate)++;
				}
			}
			else if(((*strstate)%3) == 0)
			{
				if(fileline!=9 && fileline!='\n')
				{
					token[(*tokenidx)++]=fileline;
				}
				else
				{
					token[(*tokenidx)-1]=0;
					
					*tokenidx=0;
					(*strstate)++;
				}
			}
}


//alarm對tag為一對多關係
//依所得的資料並無此關係定義, alarm ruletype=1所偵查的接點資料, 應該在程式中
void AlarmSystem::InitAlarm(char *filename, char *extfilename)
{
	FILE *f=fopen(filename,"r");
	if(f==0)
		printf("fail open alarm file:%s\n",filename);
		
	char fileline[1024];
	char token[20];
	int i,j=0, countAlarm=0;
	int strstate=0;
	datasnmp file;
	memset(&file, 0, sizeof(datasnmp));
	
	ruleextcol.Init(extfilename);
	
	while(fgets(fileline,1024,f) != NULL)
	{
		if(strstr(fileline,"array") != 0)
			countAlarm++;
	}
	Max_AlarmCnt = countAlarm;
	pAlarmCol = new AlarmContent[Max_AlarmCnt];
	memset(pAlarmCol, 0, sizeof(AlarmContent)*Max_AlarmCnt);
	countAlarm=0;
	fseek(f, 0, SEEK_SET);
	while(fgets(fileline,1024,f) != NULL)
	{
		strstate=0;
		for(i=0;i<strlen(fileline);i++)
		{
			if(strstate == 0)
			{
				if(fileline[i]>='!' && fileline[i]<='~')
				{
					if(fileline[i] == '}')
					{
						strstate=999;//this alarm setting is ended
						break;
					}
					token[j++]=fileline[i];
					strstate=1;
				}
			}
			else if(strstate == 1)
			{
				if(fileline[i]!=' ' && fileline[i]!=9)
				{
					token[j++]=fileline[i];
				}
				else
				{
					token[j]=0;
					if(strcmp(token, "array") == 0)
					{
						j=0;
						strstate=997;
						break;
					}
					else if(strcmp(token, "ip1") == 0)
					{
						strstate=2;//get ip1
						j=0;
					}
					else if(strcmp(token, "ip2") == 0)
					{
						strstate=5;//get ip1
						j=0;
					}
					else if(strcmp(token, "ip3") == 0)
					{
						strstate=8;//get ip1
						j=0;
					}
					else if(strcmp(token, "ip4") == 0)
					{
						strstate=11;//get ip1
						j=0;
					}
					else if(strcmp(token, "ruletype") == 0)
					{
						strstate=14;//get ip1
						j=0;
					}
					else if(strcmp(token, "interval") == 0)
					{
						strstate=17;//get ip1
						j=0;
					}
					else if(strcmp(token, "delay") == 0)
					{
						strstate=20;//get ip1
						j=0;
					}
					else if(strcmp(token, "startTime") == 0)
					{
						strstate=23;//get ip1
						j=0;
					}
					else if(strcmp(token, "stopTime") == 0)
					{
						strstate=26;//get ip1
						j=0;
					}
					else if(strcmp(token, "mainValue") == 0)
					{
						strstate=29;//get ip1
						j=0;
					}
					else if(strcmp(token, "mainThreshold1") == 0)
					{
						strstate=32;//get ip1
						j=0;
					}
					else if(strcmp(token, "mainThreshold2") == 0)
					{
						strstate=35;//get ip1
						j=0;
					}
					else if(strcmp(token, "mainThreshold3") == 0)
					{
						strstate=38;//get ip1
						j=0;
					}
					else if(strcmp(token, "concern1") == 0)
					{
						strstate=41;//get ip1
						j=0;
					}
					else if(strcmp(token, "concern2") == 0)
					{
						strstate=44;//get ip1
						j=0;
					}
					else if(strcmp(token, "concern3") == 0)
					{
						strstate=47;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairValue1") == 0)
					{
						strstate=50;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairValue2") == 0)
					{
						strstate=53;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairValue3") == 0)
					{
						strstate=56;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairThreshold1") == 0)
					{
						strstate=59;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairThreshold2") == 0)
					{
						strstate=62;//get ip1
						j=0;
					}
					else if(strcmp(token, "pairThreshold3") == 0)
					{
						strstate=65;//get ip1
						j=0;
					}
					else if(strcmp(token, "msg1") == 0)
					{
						strstate=68;//get ip1
						j=0;
					}
					else if(strcmp(token, "msg2") == 0)
					{
						strstate=71;//get ip1
						j=0;
					}
					else if(strcmp(token, "msg3") == 0)
					{
						strstate=74;//get ip1
						j=0;
					}
					
					else
					{//unknown key, skip this line
						j=0;
						break;
					}
				}
			}
			else
			{
				getValue(fileline[i],token,&j,&file, &strstate);
				switch(strstate)
				{
				case 4:
					strcpy(file.ip1,token);
					strstate=0;
					break;
				case 7:
					strcpy(file.ip2,token);
					strstate=0;
					break;
				case 10:
					strcpy(file.ip3,token);
					strstate=0;
					break;
				case 13:
					strcpy(file.ip4,token);
					strstate=0;
					break;
				case 16:
					strcpy(file.ruletype,token);
					strstate=0;
					break;
					case 19:
					strcpy(file.interval,token);
					strstate=0;
					break;
					case 22:
					strcpy(file.delay,token);
					strstate=0;
					break;
					case 25:
					strcpy(file.starttime,token);
					strstate=0;
					break;
					case 28:
					strcpy(file.stoptime,token);
					strstate=0;
					break;
					case 31:
					strcpy(file.mainvalue,token);
					strstate=0;
					break;
					case 34:
					strcpy(file.mainThreshold1,token);
					strstate=0;
					break;
					case 37:
					strcpy(file.mainThreshold2,token);
					strstate=0;
					break;
					case 40:
					strcpy(file.mainThreshold3,token);
					strstate=0;
					break;
					case 43:
					strcpy(file.concern1,token);
					strstate=0;
					break;
					case 46:
					strcpy(file.concern2,token);
					strstate=0;
					break;
					case 49:
					strcpy(file.concern3,token);
					strstate=0;
					break;
					case 52:
					strcpy(file.pairValue1,token);
					strstate=0;
					break;
					case 55:
					strcpy(file.pairValue2,token);
					strstate=0;
					break;
					case 58:
					strcpy(file.pairValue3,token);
					strstate=0;
					break;
					case 61:
					strcpy(file.pairThreshold1,token);
					strstate=0;
					break;
					case 64:
					strcpy(file.pairThreshold2,token);
					strstate=0;
					break;
					case 67:
					strcpy(file.pairThreshold3,token);
					strstate=0;
					break;
					case 70:
					strcpy(file.msg1,token);
					strstate=0;
					break;
					case 73:
					strcpy(file.msg2,token);
					strstate=0;
					break;
					case 76:
					strcpy(file.msg3,token);
					strstate=0;
					break;
				}
			}
		}
		if(strstate == 997)
		{
			char *colon=strchr(fileline,':');
			char *space=strchr(colon+2,' ');
			char *bigquote=strchr(colon+2,'{');
			if(space !=0)
			{
				memcpy(file.name, colon+2, space-colon-2);
				file.name[space-colon-2]=0;
			}
			else
			{
				memcpy(file.name, colon+2, bigquote-colon-2);
				file.name[bigquote-colon-2]=0;
			}
			strstate=0;
		}
		else if(strstate == 999)
		{
			strstate=0;
			
			//讀取另一個檔案，得到extended資料  oid, acktype, notifytimes(排列次序一樣)
			
			pAlarmCol[countAlarm].Init(file);
			memset(&file, 0, sizeof(datasnmp));
			countAlarm++;
		}
	}

	fclose(f);

}

U8 Rule::GetAlarmValue(U8 *pu8State)
{
	if(cmnd[0].name[0]!=0)
		TagSystemAdaptor::GetValue(cmnd[0],pu8State);
			
	if(cmnd[1].name[0]!=0)		
		TagSystemAdaptor::GetValue(cmnd[1],pu8State);

	return 1; //success
}

void AlarmContent::Execute()
{
	Rule *it = &rule;
	do
	{
		CheckExecuteSession(it);
		it->Execute();
	}while((it=it->next)!=NULL);
}

AlarmContent::~AlarmContent()
{
	Rule *r;
	r=&rule;
	if(r->next==0)
		return;

	do
	{
		r=r->next;
	}while(r->next != 0);
	
	while((r=r->prev) != 0)
	{
		free(r->next);
	}
}

int AlarmContent::CheckAlarmTiming()
{
		if(pu8StartTime[0] == 2)
		{
			struct timeval tv;
			
			time_t timep, timestart, timestop, timedayoflastsec;
			struct tm *p, startbound, stopbound, dayoflastsec;
			time(&timep);
			if((timep - LastTimeStamp) < u32Freq)
			{
				return 0;
			}
			LastTimeStamp = timep;
			
			p=localtime(&timep);
			
			startbound.tm_year=p->tm_year;
			startbound.tm_mon=p->tm_mon;
			startbound.tm_mday=p->tm_mday;
			startbound.tm_wday=p->tm_wday;
			startbound.tm_hour=pu8StartTime[5];
			startbound.tm_min=pu8StartTime[6];
			startbound.tm_sec=0;
			
			stopbound.tm_year=p->tm_year;
			stopbound.tm_mon=p->tm_mon;
			stopbound.tm_mday=p->tm_mday;
			stopbound.tm_wday=p->tm_wday;
			stopbound.tm_hour=pu8StopTime[5];
			stopbound.tm_min=pu8StopTime[6];
			stopbound.tm_sec=59;
			
			
			timestart = mktime(&startbound);
			timestop = mktime(&stopbound);
			//printf("%d %d %d\n",timep,timestart,timestop);
			if(timep>=timestart && timep<=timestop)
			{//check ok, execute
				return 1;
			}
			else
				return 0;
		}
		return 0;
}

void AlarmContent::CheckExecuteSession(Rule *r)
{
	time_t timep, timedayoflastsec;
	struct tm *p, dayoflastsec;
	time(&timep);
	p=localtime(&timep);
	
	if(pu8StartTime[0] == 2)
	{
			dayoflastsec.tm_year=p->tm_year;
			dayoflastsec.tm_mon=p->tm_mon;
			dayoflastsec.tm_mday=p->tm_mday;
			dayoflastsec.tm_wday=p->tm_wday;
			dayoflastsec.tm_hour=23;
			dayoflastsec.tm_min=59;
			dayoflastsec.tm_sec=59;
			timedayoflastsec = mktime(&dayoflastsec);
				
			if(r->u32LastFireTimeStamp >timedayoflastsec)
			{//it's a new session
				r->state=0;printf("refresh new session\n");
				r->firecount=0;
			}
	}
}

// 執行 Alram Event
void AlarmSystem::DispatchAlarmEvent()
{
	int i;
	for(i=0; i<Max_AlarmCnt; i++)
	{
//		if(pAlarmCol[i].u8Enable == 1 && (1/*time checking algorithm*/))//檢查是否達到執行條件
		if(pAlarmCol[i].CheckAlarmTiming())
		{printf("alarm timeing is ok(%d)\n",i);
			pAlarmCol[i].Execute();
		}
		else
		{
			//printf("no exec\n");
		}
	}
	
	/*
	// Check alarm是否超時
	if(CheckAlarmOvertime())
	{ 
		//overtime 回復工作初始
	}
	else
	{	
//	actlst = actlst->pNext;
	}
	// 暫停 5 ms
	//usleep(5);
	*/
}



void Rule::Execute()
{
	char cc[100];
	U8 u8State=0;
	int sockfd;
	//FILE *f=fopen("./fire.txt","w");
	GetAlarmValue(&u8State);

	switch(ext->u8AckType)
	{
		case 1:
		case 2:
			if(u8State != 1)
			{//communication error bypass this time
				printf("commu error bypass this alarm\n");
				return;
			}
			
			if(cond.Execute() == 1)
			{	
				if(state ==0)
				{
					time_t timep;
					time(&timep);
					
					state=1;
					//sprintf(cc,"fire snmp:%s->%s\n",msg, ext->oid);
					printf("fire snmp:%s->%s to %s\n",msg, ext->oid,sOidIp);
					FireOID::InitUDPCommu(&sockfd,sOidIp);
					FireOID::SendSNMPTrap(&sockfd,ext->oid, msg);
					FireOID::CloseUDPCommu(&sockfd);
					//fwrite(cc,1,strlen(cc),f);
					firecount++;
					u32LastFireTimeStamp = timep;
				}
				else if(firecount < ext->u32NotifyTimes)
				{
					time_t timep;
					time(&timep);
					
					//sprintf(cc,"fire snmp1:%s->%s\n",msg, ext->oid);
					printf("fire snmp1:%s->%s %d %d to %s\n",msg, ext->oid,firecount, ext->u32NotifyTimes,sOidIp);
					FireOID::InitUDPCommu(&sockfd,sOidIp);
					FireOID::SendSNMPTrap(&sockfd,ext->oid, msg);
					FireOID::CloseUDPCommu(&sockfd);
					//fwrite(cc,1,strlen(cc),f);
					firecount++;
					u32LastFireTimeStamp = timep;
				}
				else
				{
					//sprintf(cc,"alarm but no fire\n");
					printf("alarm but no fire\n");
					//fwrite(cc,1,strlen(cc),f);
				}
			}
			else
			{printf("reset:%s\n",msg);
				firecount=0;
				state=0;
			}
			break;
		
		case 4:
			if(u8State != 1)
			{
				printf("fire snmp:commu error %s %d\n",msg, u8State);
				FireOID::InitUDPCommu(&sockfd,sOidIp);
				FireOID::SendSNMPTrap(&sockfd,ext->oid, msg);
				FireOID::CloseUDPCommu(&sockfd);
			}
			break;
  }
	//fclose(f);
}

Rule::~Rule()
{
	free(cmnd[0].pu8ParaBuf);
	free(cmnd[1].pu8ParaBuf);

	Condition *c;
	c=&cond;
	if(c->next==0)
		return;

	do
	{
		c=c->next;
	}while(c->next != 0);
	
	while((c=c->prev) != 0)
	{
		free(c->next);
	}

//	free(cmnd);
}

RuleExt *RuleExtCol::FindRuleExt(char *name, U8 idx)
{
	for(int i=0;i<MAX_RuleExtCol; i++)
	{
		
		if(strcmp(pRuleExtCol[i].name, name)==0 && idx ==pRuleExtCol[i].u8Ruldidx)
			return pRuleExtCol+i;
	}
	printf("RuleExt nothing found!%s,%d\n",name,idx);
	return 0;
}

void RuleExtCol::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open alarmext file:%s\n",filename);
		
	char fileline[1024];

	char **out;
	int i, j=0;
	out=(char**)malloc(4 * RuleExtCol::fieldcount);
	for(i=0;i<RuleExtCol::fieldcount;i++)
	{
		out[i]=(char*)malloc(200);
	}
  
  MAX_RuleExtCol = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
  
	pRuleExtCol = (RuleExt*)malloc(sizeof(RuleExt)*MAX_RuleExtCol);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring1(fileline, ",", RuleExtCol::fieldcount, &out);


		strcpy(pRuleExtCol[j].name, out[0]);
	  pRuleExtCol[j].u8Ruldidx=atoi(out[1]);
	  strcpy(pRuleExtCol[j].oid,out[2]);
	  pRuleExtCol[j].u8AckType=atoi(out[3]);
	  pRuleExtCol[j].u32NotifyTimes=atoi(out[4]);
		//printf("ruleext:%s %d %s %d %d\n", pRuleExtCol[j].name, pRuleExtCol[j].u8Ruldidx, pRuleExtCol[j].oid, pRuleExtCol[j].u8AckType, pRuleExtCol[j].u32NotifyTimes);
		j++;		
  }
  
  for(i=0;i<RuleExtCol::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}

RuleExtCol::~RuleExtCol()
{
	free(pRuleExtCol);
}
