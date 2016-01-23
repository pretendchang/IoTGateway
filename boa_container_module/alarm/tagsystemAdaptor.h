#ifndef  _TAGSYSTEMADAPTOR_H_
#define  _TAGSYSTEMADAPTOR_H_

#include "alarm.h"

class TagSystemAdaptor
{
	public:
		static int SetValue(TagObject obj);
		static float *GetValue(TagObject obj, U8 *u8State);
};

#endif
