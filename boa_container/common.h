#ifndef _COMMON_H_
#define _COMMON_H_
#include  "typedef.h"
#include <stdio.h>
typedef float ParameterType ;
typedef struct _TagObject
{
	U32 u32deviceid;//平台定義的device id
	char *name;
	void *pTag;//指到tagsystem層的command物件
	U8 *pu8ParaBuf;//interface新定介面取得buf,由tagsystem malloc/free  暫時轉為float(ParameterType)型態，配合interface的定義
}TagObject;

class arraylist
{
	class element
	{
		public:
		void *para;
		element *next;
		element *prev;
		element()
		{
			para=0;
			next=0;
			prev=0;
		}
	};
	public:
	element *curr;
	element *head;
	int size;
	
	public:
		arraylist()
		{
			curr=0;
			head=0;
			size=0;
		}
		~arraylist()
		{
			element *elm;
			while((elm=curr->prev)!=0)
			{
				delete elm->next;
			}
			delete head;
		}
		
		void push(void *value)
		{
			if(size==0)
			{
				head = new element();
				head->para=value;
				curr = head;	
			}
			else
			{
				element *elm;
				curr->next = new element();
				elm = curr;
				curr = curr->next;
				curr->para=value;
				curr->prev=elm;
			}
			size++;	
			
		}
		void *pop(int index)
		{
			element *elm;
			elm=head;
			for(int i=0;i<index;i++)
			{
				elm=elm->next;
			}
			return elm->para;
		}
};
void *Execute(char *modulename, char *functionname, void *para);
void ExecuteTagMultiBuf1(U32 u32Deviceid, char *tagname, U8 *buf, U8 rwtype, U8 *u8State);
void ExecuteTagMultiBuf2(U32 u32Deviceid, char *tagname, U8 *buf, U8 *u8State);
float* GetTag(U32 deviceid, char *tagname, U8 *buf, U8 *u8State);
int SetTag(U32 deviceid, char *tagname, U8 *buf);
#endif
