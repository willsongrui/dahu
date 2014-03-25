#ifndef __log__
#define __log__
#include <sys/time.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

class CLOG
{
public:
	CLOG(std::string);
	~CLOG();
	void ERROR(const char* fmt,...);
	void LOG(const char* fmt,...);
	void write_to_log(const char* type, const char* fmt, va_list arg);
	int logFd;
	struct timeval tv;
	struct timezone tz;
private:
	char m_asctime[32];
	char* getTime()
	{
		struct tm *ptr;
		time_t t;
		time(&t);
		ptr = localtime(&t);
		strftime(m_asctime, 100, "%H:%M:%S",ptr);
		return m_asctime;

	}
};
#endif