
#include "Device.h"
#include "Command.h"
#include "Action.h"
#include "util.h"

extern ConnectionCollection ConnCol;
extern NodeCollection NodeCol;
extern ActionCollection actCol;
extern CommandCollection cmndCol;
extern WebDeviceCollection WebCol;

extern U32 u32ActionLstPtr[1000];
extern U32 u32ActionLstPtrTop;

#define Parameter_Buf_Len 50//@可放置的參數數目

void* FindCommandObj(U32 _webdevid, char *cmndname)//devid: 平台網站系統的devid
{
//	NodeInfo *node = NodeCol.FindByWebDevid(_devid);
	U32 devid = WebCol.FindDeviceId(_webdevid);
	return cmndCol.FindCommandObj(devid, cmndname);
}

typedef struct _Para
{//勝強所定義的傳輸介面
	char key[40];
	float value;
}Para;

//只能支援到50個參數讀回
void* GetAllTagStatus(U32 _webdevid, U32 *len, U8 *u8State)//devid: 平台網站系統的devid
{
//	NodeInfo *node = NodeCol.FindByWebDevid(_devid);
	U32 devid = WebCol.FindDeviceId(_webdevid);
	Para *ret;
	char devstatusname[30];
	U32 u32Cmndbuf[Parameter_Buf_Len];//@由1開始可定義到49
	U32 i=0;
	*len = 0;
	
	sprintf(devstatusname, "_dev%d", devid);
	Command *cmnd = (Command*)cmndCol.FindCommandObj(devid, devstatusname);
	
	if(cmnd == 0)
		return 0;

	*u8State=cmnd->Execute(u32Cmndbuf);
	if((*u8State) != 1)
		return 0;
	
	*len = cmnd->GetActionCount();

	ret = (Para*)malloc(sizeof(Para) * (*len));//外部系統會釋放
	
	
	
	ActionList *pActlst = cmnd->GetActionList();
	while(pActlst != NULL)
	{
		strcpy(ret[i].key, pActlst->GetAction()->tagname);
		ret[i].value = *(float*)(u32Cmndbuf+i);
		pActlst = pActlst->GetNext();
		i++;
	}
	return ret;
}

//u32ValBuf依照介面的定義，應該只會有一個元素
//內部運作，固定產生50*4 byte長度的記憶體
void ExecuteCommand(U32 *u32ValBuf, void *pc, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@由1開始可定義到49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, 4);//只有一個元素
	*u8State=cmnd->Execute(u32Cmndbuf);
	
	memcpy(u32ValBuf, u32Cmndbuf, 4);//只有一個元素
}

//u32ValBuf依照介面的定義，應該只會有一個元素
//內部運作，固定產生50*4 byte長度的記憶體
void ExecuteCommand1(U32 *u32ValBuf, void *pc, ActionType rwtype, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@由1開始可定義到49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, 4);//只有一個元素
	
	*u8State=cmnd->Execute(u32Cmndbuf, rwtype);
	memcpy(u32ValBuf, u32Cmndbuf, 4);//只有一個元素
}

//u32ValBuf記憶體長度為u8ValBufLen
void ExecuteCommand2(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, ActionType rwtype, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@由1開始可定義到49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, u8ValBufLen);
	
	*u8State=cmnd->Execute(u32Cmndbuf, rwtype);
	memcpy(u32ValBuf, u32Cmndbuf, u8ValBufLen);
}

//ActionType由設定檔指定，使用於Schedule功能
//u32ValBuf記憶體長度為u8ValBufLen
//跳過middle層記憶體對應，直接由上層的記憶體接取資料
void ExecuteCommand3(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	*u8State=cmnd->Execute(u32ValBuf);
}

//for action module usage
void ExecuteCommand4(U32 devid, char *cmndname, U32 *u32ValBuf, U8 u8ValBufLen, ActionType rwtype, U8 *u8State)
{
	Command *pc = (Command*)cmndCol.FindCommandObj(devid, cmndname);
	ExecuteCommand2(u32ValBuf, u8ValBufLen, pc, rwtype, u8State);
}

void FreeSystem()
{
	for(int i=0; i<u32ActionLstPtrTop; i++)
	{
		free((void*)u32ActionLstPtr[i]);
	}
}

void InitSystem(char *connfile, char *devicefile, char *actionfile, char *commandfile, char *webdevfile)
{
	ConnCol.Init(connfile);
	NodeCol.Init(devicefile);
	actCol.Init(actionfile);
  cmndCol.Init(commandfile);
  WebCol.Init(webdevfile);
}



typedef struct _tagsystem
{
	void* (*FindCommandObj)(U32 _devid, char *cmndname);
	void  (*ExecuteCommand)(U32 *u32ValBuf, void *pc, ActionType rwtype, U8 *u8State);
	void  (*FreeSystem)();
	void  (*InitSystem)(char *connfile, char *devicefile, char *actionfile, char *commandfile, char *webdevfile);
	void* (*GetAllTagStatus)(U32 _devid, U32 *len, U8 *u8State);
	
	void  (*ExecuteCommandM)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, ActionType rwtype, U8 *u8State);
	
	void  (*ExecuteCommandM1)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U8 *u8State);//設計給log用
	//Schedule功能使用，跳過middle層記憶體對應，直接由上層的記憶體接取資料
	//ActionType由設定檔指定
}tagsystem;

extern "C"
{
	void dllmain(void* para)//first dword:  
	{
		tagsystem *interface = (tagsystem*)para;
		interface->FindCommandObj = FindCommandObj;
	  interface->ExecuteCommand = ExecuteCommand1;
	  interface->FreeSystem = FreeSystem;
	  interface->InitSystem = InitSystem;
	  interface->GetAllTagStatus = GetAllTagStatus;
	  interface->ExecuteCommandM = ExecuteCommand2;
	  interface->ExecuteCommandM1 = ExecuteCommand3;
	}
}
