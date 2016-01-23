
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

#define Parameter_Buf_Len 50//@�i��m���ѼƼƥ�

void* FindCommandObj(U32 _webdevid, char *cmndname)//devid: ���x�����t�Ϊ�devid
{
//	NodeInfo *node = NodeCol.FindByWebDevid(_devid);
	U32 devid = WebCol.FindDeviceId(_webdevid);
	return cmndCol.FindCommandObj(devid, cmndname);
}

typedef struct _Para
{//�ӱj�ҩw�q���ǿ餶��
	char key[40];
	float value;
}Para;

//�u��䴩��50�ӰѼ�Ū�^
void* GetAllTagStatus(U32 _webdevid, U32 *len, U8 *u8State)//devid: ���x�����t�Ϊ�devid
{
//	NodeInfo *node = NodeCol.FindByWebDevid(_devid);
	U32 devid = WebCol.FindDeviceId(_webdevid);
	Para *ret;
	char devstatusname[30];
	U32 u32Cmndbuf[Parameter_Buf_Len];//@��1�}�l�i�w�q��49
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

	ret = (Para*)malloc(sizeof(Para) * (*len));//�~���t�η|����
	
	
	
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

//u32ValBuf�̷Ӥ������w�q�A���ӥu�|���@�Ӥ���
//�����B�@�A�T�w����50*4 byte���ת��O����
void ExecuteCommand(U32 *u32ValBuf, void *pc, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@��1�}�l�i�w�q��49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, 4);//�u���@�Ӥ���
	*u8State=cmnd->Execute(u32Cmndbuf);
	
	memcpy(u32ValBuf, u32Cmndbuf, 4);//�u���@�Ӥ���
}

//u32ValBuf�̷Ӥ������w�q�A���ӥu�|���@�Ӥ���
//�����B�@�A�T�w����50*4 byte���ת��O����
void ExecuteCommand1(U32 *u32ValBuf, void *pc, ActionType rwtype, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@��1�}�l�i�w�q��49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, 4);//�u���@�Ӥ���
	
	*u8State=cmnd->Execute(u32Cmndbuf, rwtype);
	memcpy(u32ValBuf, u32Cmndbuf, 4);//�u���@�Ӥ���
}

//u32ValBuf�O������׬�u8ValBufLen
void ExecuteCommand2(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, ActionType rwtype, U8 *u8State)
{
	Command *cmnd = (Command*)pc;
	U32 u32Cmndbuf[Parameter_Buf_Len];//@��1�}�l�i�w�q��49
	memset(u32Cmndbuf, 0, Parameter_Buf_Len*sizeof(U32));
	memcpy(u32Cmndbuf, u32ValBuf, u8ValBufLen);
	
	*u8State=cmnd->Execute(u32Cmndbuf, rwtype);
	memcpy(u32ValBuf, u32Cmndbuf, u8ValBufLen);
}

//ActionType�ѳ]�w�ɫ��w�A�ϥΩ�Schedule�\��
//u32ValBuf�O������׬�u8ValBufLen
//���Lmiddle�h�O��������A�����ѤW�h���O���鱵�����
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
	
	void  (*ExecuteCommandM1)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U8 *u8State);//�]�p��log��
	//Schedule�\��ϥΡA���Lmiddle�h�O��������A�����ѤW�h���O���鱵�����
	//ActionType�ѳ]�w�ɫ��w
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
