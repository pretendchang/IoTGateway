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

void Modbus_GenTCPHeader(MODU8 *buf)
{
	buf[0] = buf[1] = buf[2] = buf[3] = buf[4] = 0;
}

void Modbus_GenReadTelegram(PLCIOCmnd * cmnd, MODU8 *buf, MODU8 *u8Txlen)
{
	int i = 0;
	MODU8  u8len;
	MODU32 u32Addr;
	if(buf == NULL)
		return;
	
	Modbus_GenTCPHeader(buf);
	buf[6] = cmnd->u8StationID;
	switch(cmnd->u16LocID)
	{
		case 0:
			buf[7] = 1;
			u8len = (cmnd->u8Len);
			u8Rxlen = 10+(u8len-1)/8; 
			break;
			
		case 1:
			buf[7] = 2;
			u8len = (cmnd->u8Len);
			u8Rxlen = 10+(u8len-1)/8; 
			break;
			
		case 2:
			buf[7] = 4;
			u8len = (cmnd->u8Len)/2;
			u8Rxlen = cmnd->u8Len + 9; 
			break;
			
		case 3:
			buf[7] = 3;
			u8len = (cmnd->u8Len)/2;
			u8Rxlen = cmnd->u8Len + 9; 
			break;	
			
		case 4:
			buf[7] = 4;
			u8len = (cmnd->u8Len+cmnd->pU32Addr[1])/16+1;
			u8Rxlen = u8len*2 + 9; 
			break;
			
		case 5:
			buf[7] = 3;
			u8len = (cmnd->u8Len+cmnd->pU32Addr[1])/16+1;
			u8Rxlen = u8len*2 + 9; 
			break;				
	}
	u32Addr = cmnd->pU32Addr[0] - 1;
	buf[8] = (MODU8)((u32Addr>>8) & 0xff);
	buf[9] = (MODU8)(u32Addr & 0xff);
	buf[10] = (MODU8)((u8len>>8) & 0xff);
	buf[11] = (MODU8)(u8len & 0xff);

	buf[5] = 6;
	*u8Txlen = 12;
	
}

void Modbus_ParseReadTelegram(PLCIOCmnd * cmnd)
{
	int i;
	MODU8 *u8data, *u8Ret;
	if(cmnd->pBuf == NULL)
		return;

	u8data = pProtocol->pu8RxBuf + 9;
	u8Ret = cmnd->pBuf;
	switch(cmnd->u16LocID)
	{//len=1的狀況
	case 0:
	case 1:
		for(i=0; i<(cmnd->u8Len); i++)
		{
			u8Ret[i] = ((*(u8data+i/8))>>(i%8)) & 0x1;
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
	MODU8 *pu8Orgbuf;
	MODU8 pu8ReadBuf[20], u8ReadTxlen;
	memset(buf, 0, 2);
	
	if(cmnd->u16LocID == 5)
	{
			cmnd->u16LocID=3;
			cmnd->u8Len=2;
			pu8Orgbuf = cmnd->pBuf;
			cmnd->pBuf = buf;
			Modbus_GenReadTelegram(cmnd, pu8ReadBuf, &u8ReadTxlen);
			pProtocol->Send(cmnd->fd,pu8ReadBuf, u8ReadTxlen);
			if(pProtocol->WaitPLCResp())
			{
				Modbus_ParseReadTelegram(cmnd);
			}
			cmnd->u16LocID=5;
			cmnd->u8Len=1;
			cmnd->pBuf = pu8Orgbuf;
	}
}

void Modbus_GenWriteTelegram(PLCIOCmnd * cmnd, U8 *buf, MODU8 *u8Txlen)
{
	int i = 0;
	MODU8  u8len, u8tmpDataBuf[2];
	MODU16 *pu16tmpDatabuf;
	MODU32 u32Addr;
	if(buf == NULL)
		return;
	
	Modbus_GenTCPHeader(buf);
	buf[6] = cmnd->u8StationID;
	
	switch(cmnd->u16LocID)
	{
		case 0:
			if(cmnd->u8Len == 1)
			{
				buf[7] = 5;
				u8len = (cmnd->u8Len);
				u8Rxlen = 12; 
					
				if(cmnd->pBuf[0] & 0x1)
				{
					buf[10] = 0xff;
					buf[11] = 0;
				}
				else
				{
					buf[10] = 0;
					buf[11] = 0;
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
				buf[7] = 6;
				u8len = (cmnd->u8Len)/2;
				u8Rxlen = 12; 
					
				buf[10] = (MODU8)(cmnd->pBuf[1] & 0xff);
				buf[11] = (MODU8)(cmnd->pBuf[0] & 0xff);
			}
			break;			
		
		case 5://only support one bit write
			Modbus_GenReadDataBeforeWriteBit(cmnd, u8tmpDataBuf);
			
				if(*(cmnd->pState) == 1)
				{
					buf[7] = 6;
					pu16tmpDatabuf = (MODU16*)u8tmpDataBuf;
					switch(((*cmnd->pBuf)&0x1))
					{
						case 1:
							(*pu16tmpDatabuf) = (1<<cmnd->pU32Addr[1])|(*pu16tmpDatabuf);
							break;
						case 0:
							(*pu16tmpDatabuf) = (~(1<<cmnd->pU32Addr[1]))&(*pu16tmpDatabuf);
							break;
					}
					buf[10]=u8tmpDataBuf[1];
					buf[11]=u8tmpDataBuf[0];
					u8len = 1;
					u8Rxlen = 12; 
				}
				else
				{
					return;
				}
			
			
			break;
			
		case 4:
			//err
			break;	
	}
	u32Addr = cmnd->pU32Addr[0] - 1;
	buf[8] = (MODU8)((u32Addr>>8) & 0xff);
	buf[9] = (MODU8)(u32Addr & 0xff);

	buf[5] = 6;
	*u8Txlen = 12;

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


