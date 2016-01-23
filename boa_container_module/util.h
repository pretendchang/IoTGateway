#ifndef _UTIL_H_
#define _UTIL_H_
#include "typedef.h"
#include <stdio.h>

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

class util
{
	public:
		static int CalculateNewLineInFile(FILE *f);
		static void splitstring(char *in, char *splitter, int fieldcount, char ***out);
		static void splitstring1(char *in, char *splitter, int fieldcount, char ***out);
		static void DegPrint(char *fmt, ...);
		static U32 BCD2Dec(U32 bcd);
		static U32 Dec2BCD(U32 dec);
		static U32 exp1(U32 base, U32 time);
		static void timestamplog(char *msg);
		static float atoff(char *str);
		
		static void replace(char *str, char from, char to);
		
		static void formatValue(char **ptn_str, void *value, char *out);
		static void format(char *out, char *ptn, arraylist *v);//the element in v is 32 bit
		
};
#endif
