#ifndef _OSINTERFACE_H_
#define _OSINTERFACE_H_

#ifdef WIN32
typedef void (__cdecl *dllentry_type)(void *);
#else
typedef void (*dllentry_type)(void *);
#endif

struct localtimestruct
{
	int tm_year;
  int tm_mon;
  int tm_mday;

  int tm_hour;
  int tm_min;
  int tm_sec;
};

class OSInterface
{
	public:
		static void sleep(int);
		static void usleep(int);
		static void *GetEntry(char *filename, char* functionname);
		static void *GetEntry(void *pModuleHandle, char* functionname);
		static void *GetModuleHandle(char *filename);
		static void *CreateThread(void *pfunc, void *para);
		static void GetLocalTime(localtimestruct *pTime);
};
#endif
