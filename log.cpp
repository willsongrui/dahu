#include "log.h"
using namespace std;

CLOG::CLOG(string logFile)
{
	int fs = open(logFile.c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if(fs < 0)
	{
		printf("打开日志文件%s失败,错误原因: %s\n",logFile.c_str(),strerror(errno));
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
	va_start(arg,fmt);
	write_to_log("ERROR",fmt,arg);
	va_end(arg);
	
}
void CLOG::write_to_log(const char* type, const char* fmt, va_list arg)
{
	if(logFd < 0)
		return;
	char temp[500];
	memset(temp,0,sizeof(temp));
	vsnprintf(temp, sizeof(temp), fmt, arg);
	if(temp[strlen(temp)-1] == '\n')
		temp[strlen(temp)-1] = '\0';
	
	char buffer[500];
	
	sprintf(buffer,"[%s] %s %s\n",getTime(),type,temp);
	printf("%s",buffer);
	if(write(logFd, buffer, strlen(buffer)) <0)
	{
		printf("写日志文件失败,失败原因 : %s\n",strerror(errno));
	}
}
void CLOG::LOG(const char* fmt, ...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("LOG",fmt,arg);
	va_end(arg);

}