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

int CAgent::handle_message(string msg)
{
	log->LOG("收到消息 %s",msg.c_str());
	size_t pos = msg.find("<acpMessage");
	if(pos == string::npos)
	{
		log->ERROR("座席收到的消息格式错误(没有<acp),具体消息为 %s",msg.c_str());
		return -1;
	}
	msg = msg.substr(msg.find("<acpMessage"));

	xml_document<> doc;
	try
	{
		doc.parse<0>(msg.c_str());
		xml_node<>* root = doc.first_node();
		xml_node<>* body = root->first_node("body");
		xml_attribute<>* type = body->first_attribute("type");
		xml_attribute<>* name = body->first_attribute("name");
		xml_node<>* parameter = body->first_node("parameter");
		xml_attribute<>* code = parameter->first_attribute("code");
		xml_attribute<>* desc = parameter->first_attribute("desc");
	}
	catch (exception e)
	{
		
		log->ERROR("消息解析错误");
		return -1;
	}
	if(doc && root && body && type && name && parameter && code && desc)
	{

		log->ERROR("消息解析错误");
		return -1;
	}

	if(strcmp("response",type->value()) == 0)
	{
		if(strcmp("Initial", name->value() == 0))
		{
			
			xml_attribute<>* ip = parameter->first_attribute("ip");
			xml_attribute<>* port = parameter->first_attribute("port");
			curState = Initial;
			return signIn(string(ip->value()),atoi(port->value()));
		}
		else if(strcmp("SignIn", name->value()) == 0)
	}



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
int CAgent::initial()
{
	if(curState != LogOut)
	{
		log("ERROR, 要inital的座席不是登出状态");
		return -1;
	}
	curState = Initial_ING;
	msgToSend.push_back(message.initialRequestMsg);
	return 0;
}

int CAgent::signIn()
{
	if(curState != Initial)
	{
		log("ERROR, 要signIn的座席不是在initial状态");
		return -1;
	}
	curState = Initial_ING;
	msgToSend.push_back(message.signInRequestMsg);
	return 0;
}

int CAgent::setIdle()
{
	if(curState)
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






