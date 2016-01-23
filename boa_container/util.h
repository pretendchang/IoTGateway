#ifndef _UTIL_H_
#define _UTIL_H_
#include "../Plateform_header/typedef.h"
#include <stdio.h>
#include "common.h"

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
