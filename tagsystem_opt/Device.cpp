
#include "Device.h"
#include "OSInterface.h"
ConnectionCollection ConnCol;
NodeCollection NodeCol;
WebDeviceCollection WebCol;

NodeCollection::~NodeCollection()
{
	free(pNode);
}
	
NodeInfo * NodeCollection::Find(U32 devid)
{
	int i; 
	for(i=0; i<NodeCol.MAX_NodeCollection; i++)
	{
		if(devid == pNode[i].u32devid)
			return &pNode[i];	
	}
	return 0;
}

void NodeCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open device file:%s\n",filename);
		
	char fileline[1024];
	IDriver *idrv;
	char **out;
	int i, j=0;
	out=(char**)malloc(4 * NodeCollection::fieldcount);
	for(i=0;i<NodeCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_NodeCollection = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pNode = (NodeInfo*)malloc(sizeof(NodeInfo) * MAX_NodeCollection);
	memset(pNode, 0, sizeof(NodeInfo) * MAX_NodeCollection);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring(fileline, ",", NodeCollection::fieldcount, &out);
 
		pNode[j].u32devid = atoi(out[0]);
//		pNode[j].u32webdevid = atoi(out[4]);
		pNode[j].u8Connid = atoi(out[3]);
		pNode[j].u8Stnid = atoi(out[1]);
		
		pNode[j].conn = (void*)ConnCol.Find(pNode[j].u8Connid);
		idrv = ((Connection*)(pNode[j].conn))->GetInterface();
		pNode[j].pplc = (idrv->Driver_InitProtocol)(out[2]);
		pNode[j].idriver = idrv;
		pNode[j].handle = ((Connection*)(pNode[j].conn))->GetHandle();
		pNode[j].para = &(((Connection*)(pNode[j].conn))->para);
		j++;		
  		
  }
  
  for(i=0;i<NodeCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}

NodeInfo * NodeCollection::FindByWebDevid(U32 webdevid)
{
	int i; 
	for(i=0; i<NodeCol.MAX_NodeCollection; i++)
	{
		if(webdevid == pNode[i].u32webdevid)
			return &pNode[i];	
	}
	return 0;
}

ConnectionCollection::~ConnectionCollection()
{
	free(pConnObj);
}

void ConnectionCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open Connection file:%s\n",filename);
	char fileline[1024];
	char **out;
	int i, j=0;
	out=(char**)malloc(4 * ConnectionCollection::fieldcount);
	for(i=0;i<ConnectionCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_Connection = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pConnObj = (Connection*)malloc(sizeof(Connection) * MAX_Connection);
	memset(pConnObj, 0, sizeof(Connection) * MAX_Connection);

  while(fgets(fileline,1024,f) != NULL)
  {
  	U8 commutype;
  	U16 port;
  	char commupara[50];
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring(fileline, ",", ConnectionCollection::fieldcount, &out);

  	pConnObj[j].SetConnid(atoi(out[0]));
  	
  	commutype = (U8)atoi(out[1]);
  	pConnObj[j].SetConnType(commutype);
  	
  	strcpy(commupara, out[2]);
  	
  	port = atoi(out[3]);
  	
  	pConnObj[j].SetModuleName(out[4]);
  	
  	
  	if(commutype == 0)
  	{
  		pConnObj[j].para.CommuTypeID=0;
  		util::splitstring(commupara, ".", 4, &out);
  		//baudrate
  		if(strcmp(out[0], "115200")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=0;
  		else if(strcmp(out[0], "57600")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=1;
  		else if(strcmp(out[0], "38400")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=2;
  		else if(strcmp(out[0], "19200")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=3;
  		else if(strcmp(out[0], "9600")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=4;
  		else if(strcmp(out[0], "4800")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=5;
  		else if(strcmp(out[0], "2400")==0)
  			pConnObj[j].para.CommuType.serial.u8Baud=6;
  			
  		//data
  		if(strcmp(out[1], "8")==0)
  			pConnObj[j].para.CommuType.serial.u8Data=1;
  		else if(strcmp(out[1], "7")==0)
  			pConnObj[j].para.CommuType.serial.u8Data=0;
  			
  		//parity
  		if(strcmp(out[0], "n")==0)
  			pConnObj[j].para.CommuType.serial.u8Parity=0;
  		else if(strcmp(out[1], "e")==0)
  			pConnObj[j].para.CommuType.serial.u8Parity=1;	
  		else if(strcmp(out[2], "o")==0)
  			pConnObj[j].para.CommuType.serial.u8Parity=2;
  			
  		//stop
  		if(strcmp(out[0], "0")==0)
  			pConnObj[j].para.CommuType.serial.u8Stop=0;
  		else if(strcmp(out[1], "1")==0)
  			pConnObj[j].para.CommuType.serial.u8Stop=1;											
  				
  			
  		pConnObj[j].para.CommuType.serial.u8COM=port;	
  	}
  	else if(commutype == 1)
  	{
  		pConnObj[j].para.CommuTypeID=1;
  		pConnObj[j].para.CommuType.ethernet.u16Port=port;
  		strcpy(pConnObj[j].para.CommuType.ethernet.strIP, commupara);
  	}
  	j++;
//  	pConnObj[j].SetCommuPara(atoi(out[2]));
  }
  
  for(i=0;i<ConnectionCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
	
	for(j=0; j<MAX_Connection; j++)
	{
		if(pConnObj[j].Establish()== 0)
		{
			printf("Connection(%d) Initialization process fail\n", j+1);
		}
	}
	
  fclose(f);
}

Connection * ConnectionCollection::Find(U32 connid)
{
	int i; 
	for(i=0; i<ConnCol.MAX_Connection; i++)
	{
		if(connid == pConnObj[i].GetConnid())
			return &pConnObj[i];	
	}
	return 0;
}

U8 Connection::Establish()
{
	dllentry_type dllentry;

	switch(u8ConnType)
	{
		case 1://ethernet
			dllentry = (dllentry_type)OSInterface::GetEntry("./Plateform_eth.so", "dllmain");
 			(*dllentry)(&interface);
 			break;
 			
 		case 0://serial	
			dllentry = (dllentry_type)OSInterface::GetEntry("./Plateform.so", "dllmain");
			(*dllentry)(&interface);
 			break;
	}
	handling = (*interface.Driver_InitDriver)(&para);
	if(handling == 0)
	{
		return 0;
	}
	return 1;
}

void WebDeviceCollection::Init(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open WebDevice file:%s\n",filename);
	char fileline[1024];
	IDriver *idrv;
	char **out;
	int i, j=0;
	out=(char**)malloc(4 * WebDeviceCollection::fieldcount);
	for(i=0;i<WebDeviceCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_WebDevice = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pWebDeviceObj = (U32*)malloc(sizeof(U32) * MAX_WebDevice);
	memset(pWebDeviceObj, 0, sizeof(U32) * MAX_WebDevice);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring(fileline, ",", WebDeviceCollection::fieldcount, &out);
 
 		pWebDeviceObj[j] = atoi(out[1]);
		j++;		
  		
  }
  
  for(i=0;i<WebDeviceCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}

U32 WebDeviceCollection::FindDeviceId(U32 webdevid)
{
	int i; 
	for(i=0; i<MAX_WebDevice; i++)
	{
		if(webdevid == pWebDeviceObj[i])
		{
			return i+1;	
		}
	}
	return 0;
}

WebDeviceCollection::~WebDeviceCollection()
{
	free(pWebDeviceObj);
}
