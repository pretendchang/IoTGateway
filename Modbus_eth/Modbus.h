#ifndef _MODBUS_H_
#define _MODBUS_H_

#include "../Plateform_header/Driver.h"
#include "../Plateform_header/Protocol.h"


typedef unsigned char MODU8;
typedef unsigned short MODU16;
typedef unsigned int MODU32;

bool Modbus_CheckRespEnd();

void Modbus_Label2Cmnd(char* pLabel, MODU8 u8StnID, MODU8 u8Len,PLCIOCmnd * cmnd);
void Modbus_GenReadTelegram(PLCIOCmnd * cmnd, U8 *buf, U8 *u8Txlen);
void Modbus_ParseReadTelegram(PLCIOCmnd * cmnd);
void Modbus_GenWriteTelegram(PLCIOCmnd * cmnd, U8 *buf, MODU8 *u8Txlen);
void Modbus_ParseWriteTelegram(PLCIOCmnd * cmnd);


#endif
