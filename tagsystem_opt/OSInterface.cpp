#include "OSInterface.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#endif

void OSInterface::sleep(int v)
{//¥ð®§´X¬í
#ifdef WIN32
	::Sleep(v*1000);
#else
	::sleep(v);
#endif
}

void OSInterface::usleep(int v)
{//¥ð®§´X²@¬í
#ifdef WIN32
	::Sleep(v);
#else
	::usleep(v);
#endif
}			



void *OSInterface::GetEntry(char *filename, char *functionname)
{
	dllentry_type dllentry;
	
#ifdef WIN32
	HMODULE h = LoadLibraryA(filename);
	dllentry = (dllentry_type)GetProcAddress(h, functionname);
#else
	void *dllhandle = dlopen(filename, RTLD_LAZY);
 	dllentry = (dllentry_type)dlsym(dllhandle, functionname);
#endif	
	return (void*)dllentry;
}

void *OSInterface::GetEntry(void *pModuleHandle, char *functionname)
{
	dllentry_type dllentry;
	
#ifdef WIN32
	dllentry = (dllentry_type)GetProcAddress((HMODULE)pModuleHandle, functionname);
#else
 	dllentry = (dllentry_type)dlsym(pModuleHandle, functionname);
#endif	
	return (void*)dllentry;
}

void *OSInterface::GetModuleHandle(char *filename)
{
	dllentry_type dllentry;
	
#ifdef WIN32
	HMODULE dllhandle = LoadLibraryA(filename);
	return (void*)dllhandle;
#else
	void *dllhandle = dlopen(filename, RTLD_LAZY);
	return dllhandle;
#endif	
	
}

void *OSInterface::CreateThread(void *pfunc, void *para)
{
#ifdef WIN32
	::CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            (LPTHREAD_START_ROUTINE)pfunc,       // thread function name
            para,          // argument to thread function 
            0,                      // use default creation flags 
            NULL);
	return 0;
#else
	pthread_t thread1;
	pthread_create(&thread1, NULL, (void *(*)(void *))pfunc, para);
	return 0;
#endif
}

void GetLocalTime(localtimestruct *pTime)
{
#ifdef WIN32	

#else
	time_t timep;
	struct tm *p;

	if(pTime == 0)
	{
		printf("GetLocalTime: pTime has to be initialized at first\n");
		return;
	}

	time(&timep);
	p=localtime(&timep);
	
	pTime->tm_year=p->tm_year;
  pTime->tm_mon=p->tm_mon+1;
  pTime->tm_mday=p->tm_mday;
  pTime->tm_hour=p->tm_hour;
  pTime->tm_min=p->tm_min;
  pTime->tm_sec=p->tm_sec;
#endif
}
