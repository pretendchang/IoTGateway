#include "Command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define commandfieldcount 5

extern U32 u32ActionLstPtr[1000];
extern U32 u32ActionLstPtrTop;

extern ActionCollection actCol;

CommandCollection cmndCol;
void CommandCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	
	if(f==0)
		printf("fail open Command file:%s\n",filename);
	
	char fileline[1024];
  char *filelineidx, *prevfilelineidx;
  int i=0, j=0,cmmdidx=0;
  int commandschema[commandfieldcount]={1,0,0,1,1};
  char commandval[commandfieldcount][50];
  
  MAX_COMMAND_CNT = util::CalculateNewLineInFile(f);
  util::DegPrint("Cread: %d line\n",MAX_COMMAND_CNT);
	fseek(f, 0, SEEK_SET);
	pCmndObj = (Command*)malloc(sizeof(Command)* MAX_COMMAND_CNT);
	memset(pCmndObj, 0, sizeof(Command)* MAX_COMMAND_CNT);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	prevfilelineidx = filelineidx=fileline;
  	U32 webdevid=0;
  	U32 devid=0;
  	U32 cmndid=0;
  	U32 actid=0;
  	Action *act;
  	char cmndname[50];memset(cmndname,0,50);
  	while((filelineidx = strchr(prevfilelineidx, ',')) != NULL)
  	{
  		memcpy(commandval[i], prevfilelineidx, filelineidx-prevfilelineidx);
  		commandval[i][filelineidx-prevfilelineidx]=0;
  		switch(i)
  		{
  			case 0:		
  				webdevid = atoi(commandval[i]);
  				break;
  			case 1:
  				strcpy(cmndname, commandval[i]);
  				cmndid = FindCommand(webdevid, cmndname);
  				if(cmndid == -1)
  				{
  					pCmndObj[cmmdidx].SetDeviceid(webdevid);
  					pCmndObj[cmmdidx].SetName(cmndname);
  				}
  				break;
  				
  			case 2:
  				devid = atoi(commandval[i]);
  				break;
  				
  			case 3:	
	  			actid = actCol.FindAction(devid, commandval[i]);
	  			act = actCol.GetAction(actid);
	  			util::DegPrint("action obj(%d):%d, %s, %s, %d, %d\n",actid, act->deviceid, act->tagname, act->label, act->rw, act->len);
	  			
	  			break;		
  		}
  		prevfilelineidx = filelineidx+1;
  		i++;
  	}
  	strcpy(commandval[i], prevfilelineidx);

  	switch(i)
  	{
  		case 4:
  			if(cmndid == -1)
	  		{
	  			ActionList *actlst = (ActionList *)malloc(sizeof(ActionList));
	  			u32ActionLstPtr[u32ActionLstPtrTop++] = (U32)actlst;
	  			actlst->SetAction(act);
	  			strcpy(actlst->para, commandval[i]);
	  			
	  			pCmndObj[cmmdidx].SetActionList(actlst);
	  			pCmndObj[cmmdidx].GetActionList()->SetNext(NULL);
	//  				pCmndObj[cmmdidx].u32ActCnt = 1;
	
	  			util::DegPrint("command obj-1(%d):%d, %s %d\n",cmmdidx, pCmndObj[cmmdidx].GetDeviceid(), pCmndObj[cmmdidx].GetName(),u32ActionLstPtrTop);
	  			cmmdidx++;
	  		}
	  		else
	  		{
	  			ActionList *pactlst = pCmndObj[cmndid].GetActionList();
	  			pactlst->AddAction(act, commandval[i]);
	  			util::DegPrint("1command obj(%d):%d, %s\n",cmndid, pCmndObj[cmndid].GetDeviceid(), pCmndObj[cmndid].GetName());
	  		}
  			break;
  	}
  	j++;i=0;
  }
  fclose(f);
}

CommandCollection::~CommandCollection()
{
	free(pCmndObj);
}

U8 Command::Execute(U32 *u32ValBuf)
{//一有錯誤立刻中斷執行
		util::DegPrint("execute command obj:%d, %s\n", deviceid, name);
		ActionList *pActlst = act;
		U32 pu32WritingVal, u32WriteRawData, *pu32ReadingRawData;
		U8 u8ExecuteState=0;
		
		while(pActlst != NULL)
		{
			u8ExecuteState = 0;
			Action *pa = pActlst->GetAction();
			util::DegPrint("1rw:%d\n",pa->rw);
			switch(pa->rw)
			{
				case READ:
					/*
					GetReadingPara(pa->para, u32ValBuf, &pu32ReadingRawData);
					*pu32ReadingRawData=0;
					u8ExecuteState = pa->Execute(pu32ReadingRawData);
					*/
					u8ExecuteState = ExecuteReadCommand(pActlst, u32ValBuf, &pu32ReadingRawData);
//					float f;
//					f = HandlingRDataFormat(pu32ReadingRawData, pa->len, pa->datafmt);
//					memcpy(pu32ReadingRawData, &f, 4);
					util::DegPrint("Getdata:%f\n",*(float*)u32ValBuf);
					break;
				
				case WRITE:
					GetWritingPara(pActlst->para, u32ValBuf, &pu32WritingVal);
					HandlingWDataFormat((U16*)&pu32WritingVal, (U8*)&u32WriteRawData, pa->datafmt);util::DegPrint("writerawdata:%d %f\n",u32WriteRawData, *(float*)&u32WriteRawData);
					u8ExecuteState = pa->Execute(&u32WriteRawData, WRITE);
					break;
				case BATCH_WRITE_BIT:
					//mask指標放在u32ValBuf[0]中
					u32ValBuf[0] = (U32)pActlst->para;
					u8ExecuteState = pa->Execute(u32ValBuf, BATCH_WRITE_BIT);
					break;
				case DELAY:
					GetWritingPara(pActlst->para, u32ValBuf, &pu32WritingVal);
					HandlingWDataFormat((U16*)&pu32WritingVal, (U8*)&u32WriteRawData, pa->datafmt);util::DegPrint("usleep:%d\n",u32WriteRawData);
					u8ExecuteState = pa->Execute(u32ValBuf, DELAY);//u32WriteRawData must be unsigned integer	
					break;	
				case MODULE:
					u8ExecuteState = pa->Execute(u32ValBuf, MODULE);
//					GetWritingPara(pActlst->para, u32ValBuf, &pu32WritingVal);printf("wp:%f %f\n",*(float*)u32ValBuf,*(float*)pu32WritingVal);
//					HandlingWDataFormat((U16*)&pu32WritingVal, (U8*)&u32WriteRawData, pa->datafmt);
//					u8ExecuteState = pa->Execute(&u32WriteRawData, MODULE);//u32WriteRawData must be unsigned integer	
					break;
			}

			if(u8ExecuteState != 1)
			{
				util::DegPrint("execute command obj:%d, %s Action:%s fail\n", deviceid, name, pa->tagname);
				return u8ExecuteState;
			}
			
			pActlst = pActlst->GetNext();
		}
		return 1;
}

U8 Command::Execute(U32 *u32ValBuf, ActionType rwtype)
{//一有錯誤立刻中斷執行
		util::DegPrint("execute command obj:%d, %s\n", deviceid, name);
		ActionList *pActlst = act;
		U32 pu32WritingVal, u32WriteRawData, *pu32ReadingRawData;
		U8 u8ExecuteState=0;
		
		while(pActlst != NULL)
		{
			u8ExecuteState = 0;
			Action *pa = pActlst->GetAction();
			//util::DegPrint("1rw:%d\n",rwtype);
			if(pa->rw != READ && pa->rw != WRITE)
			{
				switch(pa->rw)
				{
					case BATCH_WRITE_BIT:
						//mask指標放在u32ValBuf[0]中
						u32ValBuf[0] = (U32)pActlst->para;
						u8ExecuteState = pa->Execute(u32ValBuf, BATCH_WRITE_BIT);
						break;
					case DELAY:
						GetWritingPara(pActlst->para, u32ValBuf, &pu32WritingVal);
						HandlingWDataFormat((U16*)&pu32WritingVal, (U8*)&u32WriteRawData, pa->datafmt);
						u8ExecuteState = pa->Execute(&u32WriteRawData, DELAY);//u32WriteRawData must be unsigned integer	
						break;	
					case MODULE:
						u8ExecuteState = pa->Execute(u32ValBuf, MODULE);//u32WriteRawData must be unsigned integer	
						break;	
				}
			}
			else
			{	
				switch(rwtype)
				{
					case READ:
						/*
						GetReadingPara(pa->para, u32ValBuf, &pu32ReadingRawData);
						*pu32ReadingRawData=0;
						u8ExecuteState = pa->Execute(pu32ReadingRawData);
						*/
						u8ExecuteState = ExecuteReadCommand(pActlst, u32ValBuf, &pu32ReadingRawData);
	//					float f;
	//					f = HandlingRDataFormat(pu32ReadingRawData, pa->len, pa->datafmt);
	//					memcpy(pu32ReadingRawData, &f, 4);
						util::DegPrint("Getdata:%f\n",*(float*)u32ValBuf);
						break;
					
					case WRITE:
						GetWritingPara(pActlst->para, u32ValBuf, &pu32WritingVal);
						HandlingWDataFormat((U16*)&pu32WritingVal, (U8*)&u32WriteRawData, pa->datafmt);util::DegPrint("writerawdata:%d %f\n",u32WriteRawData, *(float*)&u32WriteRawData);
						u8ExecuteState = pa->Execute(&u32WriteRawData,rwtype);
						break;
					case BATCH_WRITE_BIT:
						//mask指標放在u32ValBuf[0]中
						u32ValBuf[0] = (U32)pActlst->para;
						u8ExecuteState = pa->Execute(u32ValBuf,BATCH_WRITE_BIT);
						break;
				}
			}

			if(u8ExecuteState != 1)
			{
				util::DegPrint("execute command obj:%d, %s Action:%s fail\n", deviceid, name, pa->tagname);
				return u8ExecuteState;
			}
			
			pActlst = pActlst->GetNext();
		}
		return 1;
}

void Command::SetName(char *_name)
{
	strcpy(name, _name);
}

char *Command::GetName()
{
	return name;
}

void Command::PreParseEquation(U32 *inputVal, U32 **val, char *equation)
{//假設 1st一定是@  第一個運算元一定是*  第二個運算元一定是+或-
	//運算元只能是整數
	char var1[10], val1[10], val2[10];
	char *at, *plus, *minus, *mul, *div, *op1;
	int index=0;
	float ret=0;
	util::DegPrint("pre intputval:%f equa:%s\n",*(float*)inputVal, equation);
	at    = strchr(equation, '@');
	plus  = strchr(equation, '+');
	minus = strchr(equation, '-');
	mul   = strchr(equation, '*');
	div   = strchr(equation, '/');

	if(mul != NULL)
	{	
		op1 = mul;
		memcpy(var1, at, mul-at);
		var1[mul-at]=0;
	}
	else
	{
		op1=div;
		memcpy(var1, at, div-at);
		var1[div-at]=0;
	}
	index = atoi(var1+1);
	*val = inputVal+index-1;
}

void Command::ParsingEquation(U32 *inputVal, char *equation, ParameterType *ret)
{//假設 1st一定是@  第一個運算元一定是*  第二個運算元一定是+或-
	//運算元只能是整數
	char var1[10], val1[10], val2[10];
	char *at, *plus, *minus, *mul, *div, *op1;
	int index=0;
//	*ret = 0;
	
	util::DegPrint("intputval:%f equa:%s\n",*(float*)inputVal, equation);
	at    = strchr(equation, '@');
	plus  = strchr(equation, '+');
	minus = strchr(equation, '-');
	mul   = strchr(equation, '*');
	div   = strchr(equation, '/');

	if(mul != NULL)
	{	
		op1 = mul;
		memcpy(var1, at, mul-at);
		var1[mul-at]=0;
	}
	else
	{
		op1=div;
		memcpy(var1, at, div-at);
		var1[div-at]=0;
	}
	index = atoi(var1+1);
	memcpy(ret, &inputVal[index-1], 4);
//	ret = (float)inputVal[index-1];

//	op1[0]=*mul;
	
	if(plus != NULL)
	{
//		op2[0]='+';
		memcpy(val1, op1+1, plus-op1-1);
		val1[plus-op1-1]=0;
		strcpy(val2, plus+1);
		if(mul != NULL)
			(*ret) = (*ret)*((float)util::atoff(val1))+((float)util::atoff(val2));
		else
			(*ret) = (*ret)/((float)util::atoff(val1))+((float)util::atoff(val2));
	}
	else
	{
//		op2[0]='-';
		memcpy(val1, op1+1, plus-op1-1);
		val1[plus-op1-1]=0;
		strcpy(val2, minus+1);
		if(mul != NULL)
			(*ret) = (*ret)*((float)util::atoff(val1))-((float)util::atoff(val2));
		else
			(*ret) = (*ret)/((float)util::atoff(val1))-((float)util::atoff(val2));
	}
	util::DegPrint("ret:%f\n",*ret);
}

void Command::GetWritingPara(char *para, U32 *inputVal, U32 *val)//inputVal:float  val:float
{//僅實作到處理一個參數, equation和常數只能處理整數
	util::DegPrint("getWritingpara:%s\n",para);
	if(para[1] == '@')
	{//The parameter is from external system
		//Now assume there is only one parameter in the external system
		*val = *inputVal;
	}
	else if(para[1] == '=')
	{//it's an euqation
		util::DegPrint("it's an equation\n");
		ParameterType f;
		ParsingEquation(inputVal, para+2, &f);
		memcpy(val, &f, sizeof(ParameterType));
	}
	else
	{
		char tmp[10];
		char *endtmp;
		float f;
		memcpy(tmp, para+1, strlen(para)-2);
		tmp[strlen(para)-2]=0;
		endtmp = tmp+strlen(tmp)-1;
		f = (float)atoi(tmp);
		memcpy(val, &f, 4);
//		*val = atoi(tmp);
		util::DegPrint("GetWritingPara:%f %x %s\n",f, (U32)*val, tmp);
	}	
}

U8 Command::ExecuteReadCommand(ActionList *actLst, U32 *inputVal, U32 **val)
{//inputVal @1的記憶體位置  *val預計用來放return資料的指標
	util::DegPrint("getReadingpara\n");
	Action *act = actLst->GetAction();
	if(actLst->para[1] == '@')
	{//The parameter is from external system
		//Now assume there is only one parameter in the external system
		int idx = atoi(actLst->para+2)-1;
		U8 ret;
		*val = inputVal+idx;
		ret = act->Execute((U32*)act->pu8ActBuf, READ);
		
		HandlingRDataFormat((U32*)act->pu8ActBuf, (ParameterType*)*val, act->len, act->datafmt);
		
		if(ret != 1)
		{//fail
			return ret;
		}
		
		return 1;
		
	}
	else if(actLst->para[1] == '=')
	{//it's an euqation
		U8 ret;
		util::DegPrint("it's an equation\n");
			
		//todo: map val with inputVal
		PreParseEquation(inputVal, val, actLst->para+2);
		ret = act->Execute((U32*)act->pu8ActBuf, READ);
		
		HandlingRDataFormat((U32*)act->pu8ActBuf, (ParameterType*)*val, act->len, act->datafmt);
//		printf("404 val:%f %d\n",*(float*)(*val),act->len);
		ParsingEquation(inputVal, actLst->para+2, (float*)*val);
		
		if(ret != 1)
		{//fail
			return ret;
		}
		
		return 1;
	}
	else
	{//ignore
		U32 tmp;
		return act->Execute(&tmp, READ);
	}
}
/*
void Command::GetReadingPara(char *para, U32 *inputVal, U32 **val)//inputVal:float  val:float
{//僅實作到處理一個參數, equation和常數只能處理整數
	util::DegPrint("getReadingpara\n");
	if(para[1] == '@')
	{//The parameter is from external system
		//Now assume there is only one parameter in the external system
		int idx = atoi(para+2)-1;
		*val = inputVal+idx;
	}
	else if(para[1] == '=')
	{//it's an euqation
		util::DegPrint("it's an equation\n");
		float f= ParsingEquation(inputVal, para+2);
		memcpy(*val, &f, 4);
	}
	else
	{//ignore
	}	
}
*/
void Command::HandlingRDataFormat(U32 *pKernelBuf, ParameterType *pUsrBuf, int len, int datafmt)//len: pbuf的長度 pbuf:user space memory
{//依照iEN上層介面的定義(GetValue用float型態來傳參數) read
	int i;
	U8 *p = (U8*)pKernelBuf;
	switch(datafmt)
	{//pbuf內部存的型態, 一定要跟datafmt一樣
		case TYPE_UINT://uint 16bit
			for(i=0; i<len/2; i++)
			{
				pUsrBuf[i] = (ParameterType)(((U16*)pKernelBuf)[i]);
			}
			break;
			
		case TYPE_INT://int 16bit
			for(i=0; i<len/2; i++)
			{
				pUsrBuf[i] = (ParameterType)(((S16*)pKernelBuf)[i]);
			}
					
			break;
			
		case TYPE_BOOL://bool
			
			for(i=0; i<len; i++)
			{
				if(p[i] & 0x1)
					pUsrBuf[i] = 1;
				else
					pUsrBuf[i]=0;
			}
			break;
			
		case TYPE_FLOAT://float
			for(i=0; i<len/4; i++)
			{
				pUsrBuf[i] = ((ParameterType*)pKernelBuf)[i];
			}
			break;
		
		case TYPE_BCD://BCD
			for(i=0; i<len/4; i++)
			{
				pUsrBuf[i] = (ParameterType)util::BCD2Dec(pKernelBuf[i]);
			}
			break;	
	}
	//free(p);
}

void Command::HandlingWDataFormat(U16 *para, U8 *pbuf, int datafmt)//para: 外部系統傳來的參數, pbuf:轉換後要跟PLC通訊的實際參數
{//依照iEN上層介面的定義(SetValue用float型態來傳參數(para為float格式)) write
	ParameterType fpara, *fbuf;
	int ipara;
	U32 u32para;
	fbuf = (ParameterType*)para;
	
	switch(datafmt)
	{//pbuf內部存的型態, 一定要跟datafmt一樣
		case TYPE_UINT://uint
			u32para = (U32)(*fbuf);
			memcpy(pbuf, &u32para, 4);
			break;
			
		case TYPE_INT://int
			ipara = (S32)(*fbuf);
			memcpy(pbuf, &ipara, 4);
			break;
			
		case TYPE_BOOL://bool
			u32para = (U32)(*fbuf);
			if(u32para & 0x1)
				*pbuf = 1;
			else
				*pbuf = 0;
			break;
			
		case TYPE_FLOAT://float
			fpara = (float)(*fbuf);
			memcpy(pbuf, &fpara, 4);
			break;
			
		case TYPE_BCD://BCD
			u32para = (U32)(*fbuf);
			*(U32*)pbuf=util::Dec2BCD(u32para);
			break;				
	}
}

U32 Command::GetActionCount()
{
	ActionList *pActlst = act;
	U32 i = 0;
	while(pActlst != NULL)
	{
		pActlst = pActlst->GetNext();
		i++;
	}
	return i;
}

Command * CommandCollection::GetCommand(U32 cmndid)
{
	return &pCmndObj[cmndid];
}

U32 CommandCollection::FindCommand(U32 _devid, char *cmndname)
{
	int i;
	for(i=0; i<MAX_COMMAND_CNT; i++)
	{
		if(pCmndObj[i].GetDeviceid() == _devid && strcmp(pCmndObj[i].GetName(), cmndname)==0)
		{
			return i;
		}
	}
	return 0xffffffff;
}

void * CommandCollection::FindCommandObj(U32 _devid, char *cmndname)
{
	int i;
	for(i=0; i<MAX_COMMAND_CNT; i++)
	{
		if(pCmndObj[i].GetDeviceid() == _devid && strcmp(pCmndObj[i].GetName(), cmndname)==0)
		{
			return (void*)&pCmndObj[i];
		}
	}
	printf("find nothing for:devid:%d cmnd:%s\n",_devid, cmndname);
	return 0;
}

void *CommandCollection::FindAllCommand(U32 _devid, U32 *len)
{
	return 0;
}
