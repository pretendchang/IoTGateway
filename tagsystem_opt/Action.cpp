
#include "Action.h"
#include "../app_header/Node.h"
#include "Device.h"
#include "Opt.h"
#include "OSInterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define actionfieldcount 6

U32 u32ActionLstPtr[1000];
U32 u32ActionLstPtrTop;

ActionCollection actCol;
extern NodeCollection NodeCol;

extern OptReadCollection optReadCol;
extern Action2OptReadCollection act2OptCol;

Action::Action()
{
	memset(this, 0, sizeof(Action));
}
		
Action::~Action()
{
	//free(pu8ActBuf);
}

U8 Action::Execute(U32 *u32ValBuf, ActionType rwtype)
{
	U8 u8State;
		U32 u32devid;
		NodeInfo *pnode;
		IDriver *pinterface;
		U32 pu32WritingVal, u32WriteRawData;
		int iRetry=3, i;
		char *pmask;
		dllentry_type entrypoint;
		while(optReadCol.bIsInitialized == 0)
		{
			OSInterface::usleep(500000);
		}
		util::DegPrint("exec action obj:%d, %s, %s, %d, %d\n",this->deviceid, this->tagname, this->label, this->rw, this->len);
		u8State = 0;
		switch(rwtype)
		{
			case READ://Read
				pnode = NodeCol.Find(this->deviceid);
				pinterface = (IDriver*)pnode->idriver;
				
				util::DegPrint("action read value from %s len:%d %x\n", this->label, this->len,(int)this->pu8ActBuf);
				//printf("action read value from %s len:%d %x %d\n", this->label, this->len,(int)this->pu8ActBuf,*pu8State);
				
//				for(i=0; i<iRetry && u8State!=1; i++)
				{//u32ValBuf需配置this->len長度
//					(*(pinterface->Driver_ReadPLC))(pnode, this->label, this->len, this->pu8ActBuf, &u8State);
					memcpy(u32ValBuf, this->pu8ActBuf, this->len);
					u8State = *pu8State;

					/*
					if(u8State == 1)
					{
//						float f;
//						f = HandlingRDataFormat(u32ValBuf, this->len);
//						memcpy(u32ValBuf, &f, 4);
						util::DegPrint("Get:%d\n",*(U16*)u32ValBuf);
					}
					else
					{
//						float f=-1;
//						memcpy(u32ValBuf, &f, 4);//依照介面的定義, 錯誤要傳-1
						util::DegPrint("ErrR:%d(%d)\n",u8State,i);
					}*/
				}
				break;
			case WRITE://Write
				pnode = NodeCol.Find(this->deviceid);
				pinterface = (IDriver*)pnode->idriver;
								
				util::DegPrint("write value:%d to %s len:%d\n",*u32ValBuf, this->label, this->len);
				for(i=0; i<iRetry && u8State!=1; i++)
				{
					(*(pinterface->Driver_WritePLC))(pnode, this->label, this->len, (U8*)u32ValBuf, &u8State);
					
					if(u8State == 1)
					{
						memcpy(this->pu8ActBuf, u32ValBuf, this->len);	
						util::DegPrint("OK_W\n");
					}
					else
						util::DegPrint("ErrW:%d\n",u8State);
				}
				break;
				
			case BATCH_WRITE_BIT://special write	
				pnode = NodeCol.Find(this->deviceid);
				pinterface = (IDriver*)pnode->idriver;
				
				//mask指標放在u32ValBuf[0]中
				pmask = (char*)malloc(strlen((char*)(u32ValBuf[0])));
				GetWritingMask((char*)(u32ValBuf[0]), pmask);
				
				util::DegPrint("write mask:%s\n",pmask);
				for(i=0; i<iRetry && u8State!=1; i++)
				{
					(*(pinterface->Driver_WritePLC2))(pnode, this->label, this->len, (U8*)&pu32WritingVal, &u8State, pmask);
					if(u8State == 1)
						util::DegPrint("OK_W2:%x\n",(U16)pu32WritingVal);
					else
						util::DegPrint("ErrW2:%d\n",u8State);
				}
				free(pmask);
				break;	
				
			case DELAY:
				util::DegPrint("sleep:%d msec\n",*u32ValBuf);
				OSInterface::usleep((*u32ValBuf)*1000);//todo
				u8State=1;
				break;	
			case MODULE:
				u8State=(pModule.EntryPoint((void*)u32ValBuf));
				break;
			 	
		}
		return u8State;
}

void Action::GetWritingMask(char *para, char *mask)
{
	memcpy(mask, para+1, strlen(para)-2);
	mask[strlen(para)-2]=0;
}

extern void ExecuteCommand4(U32 _webdevid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, ActionType rwtype, U8 *u8State);
void Action::InitModule()
{
	dllentry_type dllentry = (dllentry_type)OSInterface::GetEntry(label, "dllmain");
	pModule.ExecuteCommand = ExecuteCommand4;
	(*dllentry)(&pModule);
//	pModule = (U32)OSInterface::GetEntry(label, "EntryPoint");//todo
}

//Class ActionList 		
void ActionList::AddAction(Action *_pact, char *para)
{
	ActionList *actlst = (ActionList *)malloc(sizeof(ActionList));
	u32ActionLstPtr[u32ActionLstPtrTop++] = (U32)actlst;
	ActionList *pactLst = this;
	if(actlst == NULL)
		util::DegPrint("err malloc\n");
	
  actlst->pAct = _pact;
  strcpy(actlst->para, para);
  actlst->pNext = 0;
	while(pactLst !=  NULL && pactLst->pNext != NULL)
  {
  	pactLst = pactLst->GetNext();
  }
  pactLst->pNext = actlst;
}
		
ActionList::~ActionList()
{
	/*
	ActionList *pactLst = this->GetNext();
	if(pactLst != NULL)
	{
		~pactLst();
	}
	*/
}


//Class ActionCollection
void ActionCollection::Init(char *filename)
{
	optReadCol.Init("./optread.txt");
	act2OptCol.Init("./act2opt.txt");
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open Action file:%s\n",filename);
			
	char fileline[1024];
  char *filelineidx, *prevfilelineidx;
  int i=0, j=0;
//  int actionschema[actionfieldcount]={1,0,0,1,1};
  char actionval[actionfieldcount][50];
  
  MAX_ACTION_CNT = util::CalculateNewLineInFile(f);
  util::DegPrint("Aread: %d line\n",MAX_ACTION_CNT);
	fseek(f, 0, SEEK_SET);
	pActObj = (Action*)malloc(sizeof(Action) * MAX_ACTION_CNT);
	memset(pActObj, 0, sizeof(Action) * MAX_ACTION_CNT);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	prevfilelineidx = filelineidx=fileline;
  	while((filelineidx = strchr(prevfilelineidx, ',')) != NULL)
  	{
  		memcpy(actionval[i], prevfilelineidx, filelineidx-prevfilelineidx);
  		actionval[i][filelineidx-prevfilelineidx]=0;
  		switch(i)
  		{
  			case 0:		
  				pActObj[j].deviceid = atoi(actionval[i]);
  				break;
  			case 1:
  				strcpy(pActObj[j].tagname, actionval[i]);
  				break;
  			case 2:	
  				strcpy(pActObj[j].label, actionval[i]);
  				break;
  			case 3:	
  				pActObj[j].rw = (ActionType)atoi(actionval[i]);
  				if(pActObj[j].rw == MODULE)
  				{
  					pActObj[j].InitModule();
  				}
  				break;
  			case 4:	
  				pActObj[j].len = atoi(actionval[i]);
  				pActObj[j].pu8ActBuf = (U8*)act2OptCol.FindActionBuf(pActObj[j].deviceid, pActObj[j].tagname);
  				pActObj[j].pu8State = (U8*)act2OptCol.FindActionStateBuf(pActObj[j].deviceid, pActObj[j].tagname);
  				break;
  			case 5:	
  				pActObj[j].datafmt = (ActionDataFormat)atoi(actionval[i]);
  				break;				
  		}
  		prevfilelineidx = filelineidx+1;
  		i++;
  	}
  	strcpy(actionval[i], prevfilelineidx);
  	switch(i)
  	{
  		case 0:		
  			pActObj[j].deviceid = atoi(actionval[i]);
  			break;
  		case 1:
  			strcpy(pActObj[j].tagname, actionval[i]);
  			break;
  		case 2:	
  			strcpy(pActObj[j].label, actionval[i]);
  			break;
  		case 3:	
  			pActObj[j].rw = (ActionType)atoi(actionval[i]);
  			if(pActObj[j].rw == MODULE)
  			{
  				pActObj[j].InitModule();
  			}
  			break;
  		case 4:	
  			pActObj[j].len = atoi(actionval[i]);
  			pActObj[j].pu8ActBuf = (U8*)act2OptCol.FindActionBuf(pActObj[j].deviceid, pActObj[j].tagname);
  			break;
  		case 5:	
  				pActObj[j].datafmt = (ActionDataFormat)atoi(actionval[i]);
  				break;				
  		}
  	util::DegPrint("action obj(%d):%d, %s, %s, %d, %d %d\n",j, pActObj[j].deviceid, pActObj[j].tagname, pActObj[j].label, pActObj[j].rw, pActObj[j].len, pActObj[j].datafmt);
  	j++;i=0;
  }
  fclose(f);
}


		
ActionCollection::~ActionCollection()
{
	free(pActObj);
}
		
int ActionCollection::FindAction(U32 _devid, char *tagname)
{
	int i;
	for(i=0; i<MAX_ACTION_CNT; i++)
	{			
		if(pActObj[i].deviceid == _devid && strcmp(pActObj[i].tagname, tagname)==0)
		{
			return i;
		}
	}
	return -1;
}

Action * ActionCollection::GetAction(U32 actid)
{
	return &pActObj[actid];
}
