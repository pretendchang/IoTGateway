
#include <stdarg.h>
#include <stdio.h>
void DegPrint(char *fmt, ...)
{
#ifdef _DbgPrint_
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	printf("%s",buffer);
	va_end(args);
#endif	
}
