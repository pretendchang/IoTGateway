#include "Modbus.h"
#include <stdio.h>

MODU8 Modbus_CalculateXOR(MODU8* buf, int _byteCnt);
MODU8 u8Rxlen;

ISuperProtocol *pProtocol;//模擬類別繼承super指標
void Modbus_GenTelegram(MODU8 u8UnitID, MODU8 u8LocID, MODU32 u32Addr, MODU8 *pu8Buf, MODU8 *u8Txlen)
{
	
}

MODU16 Modbus_CalculatCRC16(MODU8 input_val,MODU16 crc_val) 
{
  MODU8 bctr;

  crc_val ^= (MODU16)input_val;
  for (bctr=0; bctr<8; ++bctr){
	 if (crc_val&0x0001) crc_val=((crc_val>>1)&0x7FFF)^0xA001;
	 else crc_val=(crc_val>>1)&0x7FFF;
  }
  return(crc_val);
}


//public

//00000 locid=0   10000 locid=1  30000 locid=2  40000 locid=3
void Modbus_Label2Cmnd(char* pLabel, MODU8 u8StnID, MODU8 u8Len,PLCIOCmnd * cmnd)
{//u8Len in byte
	char *dot;
	if(pLabel == NULL)
		return;
	if(strlen(pLabel) < 2)
		return;

	cmnd->u8StationID = u8StnID;
	if((dot=strchr(pLabel, '.')) == 0)
	{
		switch(pLabel[0])
		{
		case '0':
			cmnd->u16LocID=0;
			break;
		case '1':
			cmnd->u16LocID=1;
			break;
		case '3':
			cmnd->u16LocID=2;
			break;
		case '4':
			cmnd->u16LocID=3;
			break;
		}
		cmnd->pU32Addr[0] = atoi(pLabel+1);
		cmnd->u8Len = u8Len;
	}
	else
	{
		char RAddr[20], BAddr[20];
		memcpy(RAddr, pLabel+1, dot-pLabel-1);
		RAddr[dot-pLabel-1]=0;
		strcpy(BAddr, dot+1);
		switch(pLabel[0])
		{
		case '0':
		case '1':
			printf("label pattern error:%s\n",pLabel);
			break;
		case '3':
			cmnd->u16LocID=4;
			break;
		case '4':
			cmnd->u16LocID=5;
			break;
		}
		cmnd->pU32Addr[0] = atoi(RAddr);
		cmnd->pU32Addr[1] = atoi(BAddr);
		cmnd->u8Len = u8Len;
	}
}

void Modbus_GenReadTelegram(PLCIOCmnd * cmnd, MODU8 *buf, MODU8 *u8Txlen)
{
	int i = 0;
	MODU16 u16crc = 0xffff;
	MODU8  u8len;
	MODU32 u32Addr;
	if(buf == NULL)
		return;
	
	buf[0] = cmnd->u8StationID;
	switch(cmnd->u16LocID)
	{
		case 0:
			buf[1] = 1;
			u8len = (cmnd->u8Len);
			u8Rxlen = (cmnd->u8Len-1)/8 + 6; 
			break;
			
		case 1:
			buf[1] = 2;
			u8len = (cmnd->u8Len);
			u8Rxlen = (cmnd->u8Len-1)/8 + 6; 
			break;
			
		case 2:
			buf[1] = 4;
			u8len = (cmnd->u8Len)/2;
			u8Rxlen = cmnd->u8Len + 5; 
			break;
			
		case 3:
			buf[1] = 3;
			u8len = (cmnd->u8Len)/2;
			u8Rxlen = cmnd->u8Len + 5; 
			break;
			
		case 4:
			buf[1] = 4;
			u8len = (cmnd->u8Len+cmnd->pU32Addr[1])/16+1;
			u8Rxlen = u8len*2 + 5; 
			break;
			
		case 5:
			buf[1] = 3;
			u8len = (cmnd->u8Len+cmnd->pU32Addr[1])/16+1;
			u8Rxlen = u8len*2 + 5; 
			break;				
	}
	u32Addr = cmnd->pU32Addr[0] - 1;
	buf[2] = (MODU8)((u32Addr>>8) & 0xff);
	buf[3] = (MODU8)(u32Addr & 0xff);
	buf[4] = (MODU8)((u8len>>8) & 0xff);
	buf[5] = (MODU8)(u8len & 0xff);
	for(i=0; i<6; i++)
	{
		u16crc = Modbus_CalculatCRC16(buf[i], u16crc);
	}
	buf[6] = u16crc & 0xff;
	buf[7] = (u16crc>>8) & 0xff;
	*u8Txlen = 8;
	
}

void Modbus_ParseReadTelegram(PLCIOCmnd * cmnd)
{
	int i;
	MODU8 *u8data, *u8Ret;
	if(cmnd->pBuf == NULL)
	{
		printf("PLCIOCmnd is null");
		return;
	}

	u8data = pProtocol->pu8RxBuf + 3;
	u8Ret = cmnd->pBuf;
	switch(cmnd->u16LocID)
	{//len=1的狀況
	case 0:
	case 1:
	
		for(i=0; i<(cmnd->u8Len); i++)
		{
			u8Ret[i] = (*(u8data+i/8))>>(i%8) & 0x1;
		}
		break;

	case 2:
	case 3:
		//printf("len:%d %d",cmnd->u8Len/2,cmnd->u8Len);
		for(i=0; i<(cmnd->u8Len/2); i++)
		{
			u8Ret[2*i]   = u8data[2*i+1];
			u8Ret[2*i+1] = u8data[2*i];//printf("%x,",*(U16*)(u8Ret+(2*i)));
		}//printf("\n");
		break;
		
	case 4:
	case 5:
		for(i=0; i<(cmnd->u8Len); i++)
		{
			if(((cmnd->pU32Addr[1]+i)%16) <8)
				u8Ret[i]=(u8data[2*((cmnd->pU32Addr[1]+i)/16)+1]>>((cmnd->pU32Addr[1]+i)%16))&0x1;
			else
				u8Ret[i]=(u8data[2*((cmnd->pU32Addr[1]+i)/16)]>>(((cmnd->pU32Addr[1]+i)%16)-8))&0x1;
		}
		break;	
	}
	*(cmnd->pState) = 1;//printf("pState:%x %d\n",(int)cmnd->pState, *(cmnd->pState));
}

bool Modbus_CheckRespEnd(void)
{//printf("rxlen:%d\n",u8Rxlen);
	return (*(pProtocol->pu8Cnt) == u8Rxlen);
}

void Modbus_GenReadDataBeforeWriteBit(PLCIOCmnd * cmnd, U8 *buf)
{
	MODU16 *pu16Orgbuf = (MODU16*)cmnd->pBuf;
	MODU8 pu8ReadBuf[20], u8ReadTxlen;
	memset(buf, 0, 2);
	cmnd->pBuf = buf;
	if(cmnd->u16LocID == 5)
	{
			cmnd->u16LocID=3;
			cmnd->u8Len=2;
			Modbus_GenReadTelegram(cmnd, pu8ReadBuf, &u8ReadTxlen);
			pProtocol->Send(cmnd->fd,pu8ReadBuf, u8ReadTxlen);
			if(pProtocol->WaitPLCResp())
			{
				Modbus_ParseReadTelegram(cmnd);
			}
			cmnd->u16LocID=5;
			cmnd->u8Len=1;
	}
}

void Modbus_GenWriteTelegram(PLCIOCmnd * cmnd, U8 *buf, MODU8 *u8Txlen)
{
int i = 0;
	MODU16 u16crc = 0xffff;
	MODU8  u8len, u8tmpDataBuf[2], tmpReadBuf[20], u8tmpReadTxlen;
	MODU16 *pu16buf1, *pu16buf2;
	MODU32 u32Addr;
	MODU8 *pOrgBuf;
	if(buf == NULL)
		return;
	
	buf[0] = cmnd->u8StationID;
	
	printf("u16LocID:%d\n",cmnd->u16LocID);
	switch(cmnd->u16LocID)
	{
		case 0:
			if(cmnd->u8Len == 1)
			{
				buf[1] = 5;
				u8len = (cmnd->u8Len);
				u8Rxlen = 8; 
					
				if(cmnd->pBuf[0] & 0x1)
				{
					buf[4] = 0xff;
					buf[5] = 0;
				}
				else
				{
					buf[4] = 0;
					buf[5] = 0;
				}
			}
			break;
				
		case 1:
			//err
			break;
				
		case 2:
			//err
			break;
				
		case 3:
			if(cmnd->u8Len == 2)
			{
				buf[1] = 6;
				u8len = (cmnd->u8Len)/2;
				u8Rxlen = 8; 
					
				buf[4] = (MODU8)(cmnd->pBuf[1] & 0xff);
				buf[5] = (MODU8)(cmnd->pBuf[0] & 0xff);
			}
			break;
			
		case 5://only support one bit write
			pu16buf2 = (MODU16*)cmnd->pBuf;
			memset(u8tmpDataBuf, 0, 2);
			cmnd->pBuf = u8tmpDataBuf;
			cmnd->u16LocID=3;cmnd->u8Len=2;
			Modbus_GenReadTelegram(cmnd, tmpReadBuf, &u8tmpReadTxlen);
			pProtocol->Send(cmnd->fd,tmpReadBuf, u8tmpReadTxlen);
			if(pProtocol->WaitPLCResp())
			{
				Modbus_ParseReadTelegram(cmnd);
				if(*(cmnd->pState) == 1)
				{
					buf[1] = 6;
					pu16buf1 = (MODU16*)u8tmpDataBuf;
					switch(((*pu16buf2)&0x1))
					{
						case 1:
							(*pu16buf1) = (1<<cmnd->pU32Addr[1])|(*pu16buf1);
							break;
						case 0:
							(*pu16buf1) = (~(1<<cmnd->pU32Addr[1]))&(*pu16buf1);
							break;
					}
					buf[4]=u8tmpDataBuf[1];
					buf[5]=u8tmpDataBuf[0];
					cmnd->pBuf=(MODU8*)pu16buf2;
					cmnd->u16LocID=5;
					cmnd->u8Len=1;
					u8len = 1;
					u8Rxlen = 8; 
				}
				else
				{
				}
			}
			
			break;
			
		case 4:
			//err
			break;		
	}
	u32Addr = cmnd->pU32Addr[0] - 1;
	buf[2] = (MODU8)((u32Addr>>8) & 0xff);
	buf[3] = (MODU8)(u32Addr & 0xff);

	for(i=0; i<6; i++)
	{
		u16crc = Modbus_CalculatCRC16(buf[i], u16crc);
	}
	buf[6] = u16crc & 0xff;
	buf[7] = (u16crc>>8) & 0xff;
	*u8Txlen = 8;

}

void Modbus_ParseWriteTelegram(PLCIOCmnd * cmnd)
{
	*(cmnd->pState) = 1;
}

void Modbus_ReadPLC(PLCIOCmnd *cmnd)
{/*
	U8 buf[255];
	U8 u8Txlen = 0;
	GenReadTelegram(*cmnd, buf, &u8Txlen);
	pProtocol.*Send(buf,u8Txlen);
	if(pProtocol.*WaitPLCResp())
	{//ok
		ParseReadTelegram(cmnd->pBuf);
	}*/
}

#ifdef WIN32
__declspec(dllexport) void __cdecl dllmain(void* para)
#else
void dllmain(void* para)//first dword: ISuperProtocol得到super指標   second dword: IProtocol interface回傳this指標 
#endif
{
	U32 *xdata = (U32*)para;
	IProtocol *plc;
	pProtocol = (ISuperProtocol *)xdata[0];
	plc = (IProtocol *)xdata[1];

	plc->GenReadTelegram    = Modbus_GenReadTelegram;
	plc->CheckRespEnd       = Modbus_CheckRespEnd;
	plc->GenWriteTelegram   = Modbus_GenWriteTelegram;
	plc->Label2Cmnd         = Modbus_Label2Cmnd;
	plc->ParseReadTelegram  = Modbus_ParseReadTelegram;
	plc->ParseWriteTelegram = Modbus_ParseWriteTelegram;
}

