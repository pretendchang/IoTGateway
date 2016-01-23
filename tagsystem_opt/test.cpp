
#include "Device.h"
#include "Command.h"
#include "Action.h"

extern ConnectionCollection ConnCol;
extern NodeCollection NodeCol;
extern ActionCollection actCol;
extern CommandCollection cmndCol;

void InitSystem()
{
	ConnCol.Init("./conn.txt");

	Connection *conn1 = ConnCol.Find(1);
	conn1->Establish();

	conn1 = ConnCol.Find(2);
	conn1->Establish();

	NodeCol.Init("./device.txt");
	
	actCol.Init("./action.txt");
  cmndCol.Init("./Command.txt");	
}

int main()
{
	U32 buf;
	InitSystem();
	
	NodeInfo *node = NodeCol.FindByWebDevid(2223);
	Command * cmnd = (Command*)cmndCol.FindCommandObj(node->u32devid, "test");
	cmnd->Execute(&buf);


	return 0;
}
