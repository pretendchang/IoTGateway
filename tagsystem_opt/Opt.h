
#include <stdio.h>
#include "../Plateform_header/typedef.h"
#include "../app_header/Node.h"
#include "Device.h"

#include "util.h"

typedef struct _OptRead
{
	U32 u32OptReadId;
	
	U32 deviceid;
	char label[64];
	U8 *pu8ReadBuf;
	U32 len;//in byte
	U8 u8State;
}OptRead;

class OptReadCollection
{
	private:
		OptRead *pOptReadObj;
		static const int fieldcount=4;
		
	public:
		int MAX_OptRead_CNT;
		int bIsInitialized;
		OptReadCollection(){bIsInitialized=0;}
		void Init(char *filename);
		
		~OptReadCollection();
		
		OptRead * operator[](U32 OptReadid);	
};

class Action2OptRead
{
	public:
		OptRead *pOptRead;
		U32 u32Offset;
		U32 deviceid;
		char tagname[64];
		
};
class Action2OptReadCollection
{
	private:
		Action2OptRead *pAction2OptReadObj;
		static const int fieldcount=4;
		
	public:
		int MAX_Action2OptRead_CNT;
		Action2OptReadCollection(){}
		void Init(char *filename);
		
		~Action2OptReadCollection();
		
		void* FindActionBuf(U32 _devid, char *tagname);
		void* FindActionStateBuf(U32 _devid, char *tagname);
};
