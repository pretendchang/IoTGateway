#include "ActionModule.h"

class yangyangActionModule : public ActionModule
{
	public:
	U8 EntryPoint(void* para);
	U8 Init(void* para);
	U8 Free(void* para);
};
