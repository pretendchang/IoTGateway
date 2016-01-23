#include "typedef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/signal.h>
#include <dlfcn.h>

#include <unistd.h>
#include "Container.h"
//for cpe interface
typedef struct _CPESystem
{
	int (*SetValue)(int deviceid, char *tagname, float value);
	float (*GetValue)(int deviceid, char *tagname, U8 *u8State);
	int (*GetDevice)(int Devceid, struct _response **ReData, int *len);
	void* (*CPEThread)(void* para);
	void (*Reload)(int type, char *filename);//type=1 schedule
}CPESystem;
struct _response
{
	char aa[40];
	float value;
};

//for tagsystem interface
typedef struct _tagsystem
{
	Tag* (*FindCommandObj)(U32 _devid, char *cmndname);
	void (*ExecuteCommand)(U32 *u32ValBuf, Tag *pc, U32 rwtype, U8 *u8State);
	void (*FreeSystem)();
	void (*InitSystem)(char *connfile, char *devicefile, char *actionfile, char *commandfile, char *webdevfile);
	void* (*GetAllTagStatus)(U32 _devid, U32 *len, U8 *u8State);
	void  (*ExecuteCommandM)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U32 rwtype, U8 *u8State);
	
	void  (*ExecuteCommandM1)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U8 *u8State);
}tagsystem;

tagsystem interface;

Tag* (*FindTag)(U32 _devid, char *cmndname);
void (*ExecuteTag)(U32 *u32ValBuf, Tag *pc, U32 rwtype, U8 *u8State);//set get value
void (*FreeTagSystem)();
void (*InitTagSystem)(char *connfile, char *devicefile, char *actionfile, char *commandfile, char *webdevfile);
void* (*GetAllTagStatus)(U32 _devid, U32 *len, U8 *u8State);//getdevice
void  (*ExecuteCommandM1)(U32 *u32ValBuf, U8 u8ValBufLen, void *pc, U8 *u8State);//schedule log

//int SetValue(int deviceid, char *tagname, float value);
//float GetValue(int deviceid, char *tagname);
/*
Tag *GetTag(U32 _devid, char *cmndname)
{
	return FindTag(_devid,cmndname);
}
*/
//return: 0 for success, -1 for fail
//deviceid: 平台系統定義的deviceid
//tagname:平台系統定義的指令名稱, 對應於command.txt的第二個參數
//value:平台系統定義所要寫入的值
//此函數不做multithread的控管
int SetValue(int deviceid, char *tagname, float value)
{//0 for success op, -1 for error op

	U8 u8State=0;
	Tag *pcmnd = (Tag*)FindTag(deviceid, tagname);
	if(pcmnd == 0)
	{
		return -1;
	}
	ExecuteTag((U32*)&value, pcmnd, 1,&u8State);

	return u8State;

}

//return: -1 for fail, 其他值為自底層設備正確回傳的值
//deviceid: 平台系統定義的deviceid
//tagname:平台系統定義的指令名稱, 對應於command.txt的第二個參數
//此函數不做multithread的控管
float GetValue(int deviceid, char *tagname,U8 *u8State)
{// -1 means error operation, and the other value is the correct

	Tag *pcmnd = (Tag*)FindTag(deviceid, tagname);
	if(pcmnd == 0)
	{
		return -1;
	}
	float value;
	ExecuteTag((U32*)&value, pcmnd,0,u8State); 
	return value;
}

//deviceid: 平台系統定義的deviceid
//ReData存放該device所有資料的空間, 空間由GetDevice函數配置, 上層的函數需自行釋放空間
//len:ReData的長度
//此函數不做multithread的控管
int GetDevice(int Devceid, struct _response **ReData, int *len)
{
	U8 u8State=0;
	*ReData = (struct _response *)GetAllTagStatus(Devceid, (U32*)len,&u8State);
	return u8State;
}
//for schedule and log system usage
int ExecuteTagWithoutType(Tag *pcmnd, void *inputBuf, U8 u8buflen, U8 *u8State)
{
	ExecuteCommandM1((U32*)inputBuf, u8buflen, pcmnd,u8State);
	return 1;
}

typedef void (*dllentry_type)(void *);
void InitDriver()
{
	void *dllhandle;
	void (*dllentry)(void *);

	dllhandle = dlopen("./tagsystem.so", RTLD_LAZY);
//	dllhandle = dlopen("/home/root/steve1/tagsystem.so", RTLD_LAZY);	
 	dllentry = (dllentry_type)dlsym(dllhandle, "dllmain");	
 	(*dllentry)(&interface); 	
 	
 	InitTagSystem   = interface.InitSystem;
 	FreeTagSystem   = interface.FreeSystem;
 	ExecuteTag      = interface.ExecuteCommand;
 	FindTag         = interface.FindCommandObj;
 	GetAllTagStatus = interface.GetAllTagStatus;
 	ExecuteCommandM1 = interface.ExecuteCommandM1;

	InitTagSystem("./conn.txt", "./device.txt", "./action.txt", "./command.txt", "./webdevice.txt");
}

void FreeDriver()
{
	FreeTagSystem();
}


extern ModuleCollection ModCol;
void* CPEEntry(void* para)
{
	InitDriver();
	ModCol.LoadModule();
	pthread_t thr1;
	for(int i=0; i<ModCol.MAX_ModuleCollection; i++)
	{
		//create thread
		ModCol[i]->iInterface.Init(0);
		pthread_create(&thr1, NULL, ModCol[i]->iInterface.CPEThread, NULL);  
	}
	
	return 0;
}

int main()
{
	CPEEntry(0);
	while(1)
	{
		sleep(100);
	}
	return 0;
}

void Reload(int type, char *filename)
{//web層把type hardcode  1一定是schedule
	ModCol[type-1]->Execute("reload", filename);
}
void *Execute(char *modulename, char *functionname, void *para)
{
	for(int i=0; i<ModCol.MAX_ModuleCollection; i++)
	{
		if(strcmp(ModCol[i]->name,modulename)==0)
			return ModCol[i]->Execute(functionname, para);
	}
	return 0;
}
void ExecuteTagMultiBuf1(U32 u32Deviceid, char *tagname, U8 *buf, U8 rwtype, U8 *u8State)
{
	Tag *pcmnd = (Tag*)FindTag(u32Deviceid, tagname);
	if(pcmnd == 0)
	{
		return;
	}
	//ExecuteCommandM((U32*)buf, 0, pcmnd,rwtype,u8State);////buflen 底層有用
}

void ExecuteTagMultiBuf2(U32 u32Deviceid, char *tagname, U8 *buf, U8 *u8State)
{
	Tag *pcmnd = (Tag*)FindTag(u32Deviceid, tagname);
	if(pcmnd == 0)
	{
		return;
	}
	ExecuteCommandM1((U32*)buf, 0, pcmnd,u8State);//buflen 已經沒用
}

float* GetTag(U32 deviceid, char *tagname, U8 *buf, U8 *u8State)
{
	Tag *pcmnd = (Tag*)FindTag(deviceid, tagname);
	float value;
	if(pcmnd == 0)
	{
		*u8State=2;
		value=-1;
		memcpy(buf, &value, sizeof(float));
		return (float*)buf;
	}
	
	ExecuteTag((U32*)&value, pcmnd,0,u8State);
	memcpy(buf, &value, sizeof(float));
	return (float*)buf;
}

int SetTag(U32 deviceid, char *tagname, U8 *buf)
{
	U8 u8State=0;
	Tag *pcmnd = (Tag*)FindTag(deviceid, tagname);
	if(pcmnd == 0)
	{
		return -1;
	}
	ExecuteTag((U32*)buf, pcmnd, 1,&u8State);

	return u8State;
}
extern "C"
{
	void dllmain(void* para)//first dword:  
	{
		CPESystem *interface = (CPESystem*)para;
		interface->SetValue = SetValue;
	  interface->GetValue = GetValue;
	  interface->GetDevice = GetDevice;
	  interface->CPEThread = CPEEntry;
	  interface->Reload = Reload;
	}
	
	
}
