#include "Opt.h"
#include "OSInterface.h"
OptReadCollection optReadCol;
Action2OptReadCollection act2OptCol;
extern NodeCollection NodeCol;

extern "C"
{
	void *ReadDeviceThread(void *para)
	{
		int i=0,j=0;
		U8 u8state=0;
		while(1)
		{
			for(i=1; i<optReadCol.MAX_OptRead_CNT+1; i++)
			{
				OptRead *read = optReadCol[i];
				NodeInfo *pnode = NodeCol.Find(read->deviceid);//printf("read value from %d %s len:%d %x %x\n", read->deviceid,read->label, read->len,(int)read->pu8ReadBuf);
				IDriver *pinterface = (IDriver*)pnode->idriver;
				
				do
				{
					(*(pinterface->Driver_ReadPLC))(pnode, read->label, read->len, read->pu8ReadBuf, &u8state);
					j++;
				}while(read->u8State != 1 && j<3);
				read->u8State = u8state;
				if(read->u8State == 1)
					{
						//util::DegPrint("Get:%f\n",*(float*)read->pu8ReadBuf);
					}
					else
					{
						printf("ErrOPT:%d %s %d\n",read->deviceid,read->label, read->u8State);
					}
				
				if(i == optReadCol.MAX_OptRead_CNT)
				{
					i=0;
					optReadCol.bIsInitialized = 1;
					OSInterface::usleep(100000);
					//sleep(1);	
				}
				
				OSInterface::usleep(10000);	
			}
			
		}
	}
}


void OptReadCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open opt file:%s\n",filename);
		
	char fileline[1024];
	char **out;
	int i, j=0;

	out=(char**)malloc(4 * OptReadCollection::fieldcount);
	for(i=0;i<OptReadCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_OptRead_CNT = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pOptReadObj = (OptRead*)malloc(sizeof(OptRead) * MAX_OptRead_CNT);
	memset(pOptReadObj, 0, sizeof(OptRead) * MAX_OptRead_CNT);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring(fileline, ",", OptReadCollection::fieldcount, &out);
 
		pOptReadObj[j].u32OptReadId = atoi(out[0]);
		pOptReadObj[j].deviceid = atoi(out[1]);
		pOptReadObj[j].pu8ReadBuf = (U8*)malloc(atoi(out[3]));
		memset(pOptReadObj[j].pu8ReadBuf, 0, atoi(out[3]));
		pOptReadObj[j].len = atoi(out[3]);//len
		strcpy(pOptReadObj[j].label, out[2]);

		j++;		
  }
  
  for(i=0;i<OptReadCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
  
/*  
	struct sched_param param;
  pthread_attr_t attr_obj;
  pthread_attr_init(&attr_obj);
  pthread_attr_setschedpolicy(&attr_obj, SCHED_RR);
  param.sched_priority=99;
  pthread_attr_setschedparam(&attr_obj,&param);
  pthread_create(&thread1, &attr_obj, ReadDeviceThread, NULL);  
*/
  OSInterface::CreateThread((void*)ReadDeviceThread, NULL); 
  
}
		
OptReadCollection::~OptReadCollection()
{
	int i;
	for(i=0; i<OptReadCollection::fieldcount; i++)
	{
		free(pOptReadObj[i].pu8ReadBuf);
	}
	free(pOptReadObj);
}
		
OptRead *  OptReadCollection::operator[](U32 OptReadid)
{
	return (pOptReadObj+OptReadid-1);
}


void Action2OptReadCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open act2opt file:%s\n",filename);
		
	char fileline[1024];
	char **out;
	int i, j=0;
	out=(char**)malloc(4 * Action2OptReadCollection::fieldcount);
	for(i=0;i<Action2OptReadCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_Action2OptRead_CNT = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pAction2OptReadObj = (Action2OptRead*)malloc(sizeof(Action2OptRead) * MAX_Action2OptRead_CNT);
	memset(pAction2OptReadObj, 0, sizeof(Action2OptRead) * MAX_Action2OptRead_CNT);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring(fileline, ",", Action2OptReadCollection::fieldcount, &out);
 
		pAction2OptReadObj[j].pOptRead = optReadCol[atoi(out[0])];	
		pAction2OptReadObj[j].u32Offset = atoi(out[1]);
		pAction2OptReadObj[j].deviceid = atoi(out[2]);
		strcpy(pAction2OptReadObj[j].tagname, out[3]);

		j++;		
  }
  
  for(i=0;i<Action2OptReadCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}
		
Action2OptReadCollection::~Action2OptReadCollection()
{
	free(pAction2OptReadObj);
}
		
void* Action2OptReadCollection::FindActionBuf(U32 _devid, char *tagname)
{
	int i;
	for(i=0; i<MAX_Action2OptRead_CNT; i++)
	{
		if(pAction2OptReadObj[i].deviceid == _devid && strcmp(pAction2OptReadObj[i].tagname, tagname)==0)
		{
			return (pAction2OptReadObj[i].pOptRead->pu8ReadBuf+pAction2OptReadObj[i].u32Offset);
		}
	}
	return 0;
}

void* Action2OptReadCollection::FindActionStateBuf(U32 _devid, char *tagname)
{
	int i;
	for(i=0; i<MAX_Action2OptRead_CNT; i++)
	{
		if(pAction2OptReadObj[i].deviceid == _devid && strcmp(pAction2OptReadObj[i].tagname, tagname)==0)
		{
			return &(pAction2OptReadObj[i].pOptRead->u8State);
		}
	}
	return 0;
}


