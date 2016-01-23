#include "ActionModule.h"

class yangyangActionModule : public ActionModule
{
	public:
	void* EntryPoint(void* para);
	void* Init(void* para);
	void* Free(void* para);
};
