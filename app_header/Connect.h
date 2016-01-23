#ifndef _CONNECT_H_
#define _CONNECT_H_
#include "../Plateform_header/typedef.h"
#include "Node.h"

class Connection
{
	private:
		U32 u32Connid;
		U8  u8ConnType;
		char ModuleName[30];
		IDriver interface;
		int handling;
		
	public:
		CommuPara para;
		
		U32 GetConnid(){return u32Connid;}
		void SetConnid(U32 _id){u32Connid = _id;}
		void SetConnType(U8 _type){u8ConnType = _type;}
//		void SetCommuPara(CommuPara *_para){memcpy(&para, _para, sizeof(CommuPara));}
		void SetModuleName(char *name){strcpy(ModuleName, name);}
		IDriver *GetInterface(){return &interface;}
		int *GetHandle(){return &handling;}
		U8 Establish();
};

#endif
