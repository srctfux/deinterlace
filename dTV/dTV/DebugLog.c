
/*
* Debug logging functions.
*/
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>

static FILE *debugLog = NULL;

void
LOG(LPCSTR format, ...)
{
	DWORD systime;
	struct _timeb tb;
	struct tm *tm;
	char stamp[100];
	va_list args;

	if (debugLog == NULL)
	debugLog = fopen("dtv.txt", "w");

	if (debugLog == NULL)
	return;

	systime = timeGetTime();

	_ftime(&tb);
	tm = localtime(&tb.time);
	strftime(stamp, sizeof(stamp), "%y%m%d %H%M%S", tm);
	fprintf(debugLog, "%s.%03d(%03d)", stamp, tb.millitm, systime % 1000);

	va_start(args, format);
	vfprintf(debugLog, format, args);
	va_end(args);

	fputc('\n', debugLog);
	fflush(debugLog);
}

