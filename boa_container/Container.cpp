
#include "Container.h"
#include "util.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
typedef void (*dllentry_type)(void *);

ModuleCollection ModCol;
ModuleFunctionCollection ModFunCol;

void ModuleCollection::Read(char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f==0)
		printf("fail open device file:%s\n",filename);
		
	char fileline[1024];

	char **out;
	int i, j=0;
	out=(char**)malloc(4 * ModuleCollection::fieldcount);
	for(i=0;i<ModuleCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}

  MAX_ModuleCollection = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pModuleCol = (CustomModule*)malloc(sizeof(CustomModule) * MAX_ModuleCollection);
	memset(pModuleCol, 0, sizeof(CustomModule) * MAX_ModuleCollection);

  while(fgets(fileline,1024,f) != NULL)
  {
		void (*dllentry)(void *);
	
		fileline[strlen(fileline)-2]=0;
  	util::splitstring1(fileline, ",", ModuleCollection::fieldcount, &out);
	
		pModuleCol[j].u32Handle = (U32)dlopen(out[1], RTLD_LAZY);printf("42:%s %x\n",out[1],pModuleCol[j].u32Handle);
		strcpy(pModuleCol[j].name,out[0]);
		pModuleCol[j].u32FunctionCnt=0;
		
		//init module
	 	dllentry = (dllentry_type)dlsym((void*)pModuleCol[j].u32Handle, "dllmain");
	 	pModuleCol[j].iInterface.Execute = Execute;
	 	pModuleCol[j].iInterface.ExecuteTagMultiBuf1 = ExecuteTagMultiBuf1;
	 	pModuleCol[j].iInterface.ExecuteTagMultiBuf2 = ExecuteTagMultiBuf2;
	 	pModuleCol[j].iInterface.GetValue = GetTag;
	 	pModuleCol[j].iInterface.SetValue = SetTag;printf("46:%x\n",(U32)dllentry);
	 	(*dllentry)(&pModuleCol[j].iInterface);printf("47\n");
	 	j++;
  }
  
  for(i=0;i<ModuleCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  fclose(f);
}

void ModuleCollection::LoadModule()
{
	Read("./module.txt");
	ModFunCol.Read("./function.txt");
}

CustomModule *ModuleCollection::operator[](int index)
{
	return pModuleCol+index;
}

CustomModule *ModuleCollection::Find(char *name)
{
	for(int i=0; i<MAX_ModuleCollection; i++)
	{
		if(strcmp(name, pModuleCol[i].name) == 0)
			return &pModuleCol[i];
			
	}
}

void ModuleFunctionCollection::Read(char *filename)
{
	FILE *f = fopen(filename, "r");
	CustomModule *pModule;
	if(f==0)
		printf("fail open device file:%s\n",filename);
		
	char fileline[1024];

	char **out;
	int i, j=0;
	out=(char**)malloc(4 * ModuleFunctionCollection::fieldcount);
	for(i=0;i<ModuleFunctionCollection::fieldcount;i++)
	{
		out[i]=(char*)malloc(50);
	}
  
  MAX_ModuleFunctionCollection = util::CalculateNewLineInFile(f);
  
  fseek(f, 0, SEEK_SET);
	pFunction = (ModuleFunction*)malloc(sizeof(ModuleFunction) * MAX_ModuleFunctionCollection);
	memset(pFunction, 0, sizeof(ModuleFunction) * MAX_ModuleFunctionCollection);

  while(fgets(fileline,1024,f) != NULL)
  {
  	fileline[strlen(fileline)-2]=0;
  	util::splitstring1(fileline, ",", ModuleFunctionCollection::fieldcount, &out);
  	
  	strcpy(pFunction[j].modulename, out[0]);	
  	strcpy(pFunction[j].funcname, out[1]);
  	pModule = ModCol.Find(out[0]); 
  	pFunction[j].u32Handle = (U32)dlsym((void*)pModule->u32Handle, pFunction[j].funcname);	
  	
  	(pModule->u32FunctionCnt)++;
  	pModule->pFunction.push(&pFunction[j]);
	 	j++;
  }
  
  for(i=0;i<ModuleFunctionCollection::fieldcount;i++)
	{
		free(out[i]);
	}
	free(out);
  
  fclose(f);
}

void *CustomModule::Execute(char *functionname, void *para)
{
	for(int i=0; i<u32FunctionCnt; i++)
	{
		ModuleFunction *mFunc = (ModuleFunction *)pFunction.pop(i);
		if(strcmp(mFunc->funcname, functionname) == 0)
		{
			return mFunc->Execute(para);
		}
	}
	return 0;
}

void *ModuleFunction::Execute(void *para)
{
			void *(*dllentry)(void *);
			dllentry = (void *(*)(void *))u32Handle;
			return dllentry(para);
}
