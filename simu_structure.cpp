#include <simu_structure.h>

CConf::CConf()
{
	ipc = "";
	ctiIp = "";
	ctiPort = 0;
	agentNum = 0;
	agentId = 0;
	logFile = 0;
}
CAgent::CAgent()
{
	sock = -1;
	sockState = 0;
	totalCall = 0;
	successCall = 0;
	curState = preState = LogOut;
	while(msgToSend.empty() == false)
	{
		msgToSend.pop();
	}
	while(msgRecieved.empty() == false)
	{
		msgRecieved.pop();
	}
}
int CAgent::logIn()
{
	if(curState != LogOut)
	{
		log("要logIn的座席不是登出状态");
		return -1;
	}
	msgToSend.push_back(message.initialRequestMsg);
	return 0;
}
int CAgent::signIn()
{
	if(curState != LogIn)
	{
		log("要signIn的座席不是在logIn状态");
		return -1;
	}
	msgToSend.push_back(message.signInRequestMsg);
	return 0;
}

CLOG::CLOG(string logFile)
{
	fs = open(logFile.c_str(),O_WRONLY);
	if(fs < 0)
	{
		printf("打开日志文件%s失败,错误原因: %s",logFile.c_str(),strerror(errno));
		logFile = -1;
	}
	else
	{
		logFile = fs;
	}
}
void CLOG::ERROR(const char* fmt,...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("ERROR",fmt,arg);
	va_end();
	
}
void CLOG::write_to_log(const char* type, const char* fmt, va_list arg)
{
	if(logFile < 0)
		return;
	
	char temp[500];
	
	vsnprintf(temp, sizeof(temp), fmt, arg);
	
	temp[strlen(temp)-1] == '\0';
	char buffer[500];
	
	sprintf(buffer,"%s [%s] %s\n",type,getTimetemp,temp);
	if(write(logFile, temp, sizeof(buffer)) <0 )
	{
		printf("写日志文件失败,失败原因 : %s",strerror(errno)):
	}


}
void CLOG::LOG(const char* fmt, ...)
{
	va_list arg;
	va_start(arg,fmt);
	write_to_log("LOG",fmt,arg);
	va_end();

}






