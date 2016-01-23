#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../Plateform_header/typedef.h"
#include "../app_header/Node.h"
#include "../app_header/Connect.h"
#include "util.h"

class NodeCollection
{
	private:
		NodeInfo *pNode;
		int MAX_NodeCollection;
		static const int fieldcount=4;
		
	public:
		NodeCollection(){}
		~NodeCollection();
		void Init(char *filename);
		NodeInfo * Find(U32 devid);//由前端系統deviceid找NodeInfo物件
		NodeInfo * FindByWebDevid(U32 webdevid);//由平台網站系統deviceid找NodeInfo物件
};

class ConnectionCollection
{
	private:
		Connection *pConnObj;
		int MAX_Connection;
		static const int fieldcount=5;
	public:
		ConnectionCollection(){}
		~ConnectionCollection();
		void Init(char *filename);
		Connection * Find(U32 connid);
};

class WebDeviceCollection
{
	private:
		U32 *pWebDeviceObj;
		int MAX_WebDevice;
		static const int fieldcount=2;
	public:
		WebDeviceCollection(){}
		~WebDeviceCollection();
		void Init(char *filename);
		U32 FindDeviceId(U32 id);	
};
#endif
