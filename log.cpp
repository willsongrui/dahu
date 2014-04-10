#include <unistd.h>
#include "log.h"
using namespace std;

CLOG::CLOG(string logFile)
{
	//printf("进入CLOG的构造函数");
	unlink(logFile.c_str());
	int fs = open(logFile.c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if(fs < 0)
	{
		printf("打开日志文件%s失败,错误原因: %s\n", logFile.c_str(), strerror(errno));
		logFile = -1;
		throw(-1);
	}
	else
	{
		logFd = fs;
	}
}
CLOG::~CLOG()
{
	close(logFd);
}
void CLOG::ERROR(const char* fmt,...)
{
	va_list arg;
	va_start(arg, fmt);
	write_to_log("ERROR", fmt, arg);
	va_end(arg);
	
}

void CLOG::WARNING(const char* fmt,...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("WARNING", fmt, arg);
	va_end(arg);
	
}

void CLOG::write_to_log(const char* type, const char* fmt, va_list arg)
{
	if(logFd < 0)
		return;
	char temp[500];
	//char temp[1024];
	memset(temp,0,sizeof(temp));
	vsnprintf(temp, sizeof(temp), fmt, arg);
	if(temp[strlen(temp)-1] == '\n')
		temp[strlen(temp)-1] = '\0';
	
	char buffer[500];
	//char buffer[1024];
	snprintf(buffer, sizeof(buffer), "[%s]%s %s\n",getTime(),type, temp);
	/*
	if(strcmp(type, "LOG") != 0)
	{
		if(strcmp(type, "WARNING") != 0)
		{
			printf("%s",buffer);
		}
	}*/
	buffer[sizeof(buffer) - 2] = '\n';
	buffer[sizeof(buffer) - 1] = '\0';
	printf("%s",buffer);
	
	if(write(logFd, buffer, strlen(buffer)) <0)
	{
		printf("写日志文件失败,失败原因 : %s\n",strerror(errno));
	}
}

void CLOG::INFO(const char* fmt, ...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("INFO",fmt,arg);
	va_end(arg);

}

void CLOG::LOG(const char* fmt, ...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("LOG",fmt,arg);
	va_end(arg);

}