#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct _response
{
	char aa[40];
	float value;
}response;



typedef struct _CPESystem
{
	int (*SetValue)(int deviceid, char *tagname, float value);
	float (*GetValue)(int deviceid, char *tagname, unsigned char *u8State);
	int (*GetDevice)(int Devceid, struct _response **ReData, int *len);
	void* (*CPEThread)(void* para);
	void (*Reload)(int type, char *filename);
}CPESystem;

CPESystem interface;

#endif
