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
	
	doc.parse<0>(msg.c_str());
	xml_node<>* root = doc.first_node();
	xml_node<>* header = root->first_node("header");
	xml_node<>* body = root->first_node("body");
	xml_attribute<>* type = body->first_attribute("type");
	xml_attribute<>* name = body->first_attribute("name");
	xml_attribute<>* code = body->first_node("cause")->first_attribute("code");
	xml_attribute<>* desc = body->first_node("cause")->first_attribute("desc");
		
	if(!(doc && root && body && type && name && parameter && code && desc))
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
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			if(!(sessionID && timeStamp && cause && code && desc))
			{
				log->ERROR("收到的SignIn Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("signIn失败，失败原因%s",desc->value());
				return -1;
			}
			curState = SignIn;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
			return setIdle();
		}
		else if(strcmp("SignOut", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			if(!(sessionID && timeStamp code && desc))
			{
				log->ERROR("收到的SignOut Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("signOut失败，失败原因%s",desc->value());
				return -1;
			}
			curState = SignOut;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("SetBusy", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			xml_node<>* cause = body->first_node("cause");
			xml_attribute<>* code = cause->first_attribute("code");
			xml_attribute<>* desc = cause->first_attribute("desc");

			if(!(sessionID && timeStamp && cause && code && desc))
			{
				log->ERROR("收到的setBusy Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("setBusy失败，失败原因%s",desc->value());
				return -1;
			}
			curState = Busy;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("SetIdle", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的setIdle Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("setIdle失败，失败原因%s",desc->value());
				return -1;
			}
			curState = Idle;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("AgentReconnect", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的setIdle Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("AgentReconnect失败，失败原因%s",desc->value());
				return -1;
			}
			curState = SignIn;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("ForceIdle", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的forceIdle Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("forceIdle失败，失败原因%s",desc->value());
				return -1;
			}
			curState = Idle;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("ForceBusy", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的forceBusy Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("forceBusy 失败，失败原因%s",desc->value());
				return -1;
			}
			curState = Busy;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("ForceOut", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的forceOut Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("forceOut失败，失败原因%s",desc->value());
				return -1;
			}
			curState = SignOut;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("ReleaseCall", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的releaseCall Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("releaseCall失败，失败原因%s",desc->value());
				return -1;
			}
			//curState = SignOut;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
			return setIdle();
		}

		else if(strcmp("ForceOut", name->value()) == 0)
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			
			if(!(sessionID && timeStamp && code && desc))
			{
				log->ERROR("收到的forceOut Response错误");
				return -1;
			}
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("forceOut失败，失败原因%s",desc->value());
				return -1;
			}
			curState = SignOut;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}


	}
	else if(strcmp("event", type->value()))
	{
		if(strcmp("onForceIdle",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("onForceIdle失败，失败原因%s",desc->value()));
				return -1;
			}
			curState = Idle;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}


		else if(strcmp("OnAnserSuccess",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("onForceIdle失败，失败原因%s",desc->value()));
				return -1;
			}
			curState = Calling;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
			totalCall++;
		}

		else if(strcmp("OnAnserFailue",name->value()))
		{
			log->ERROR("通话失败,收到answer_failue消息");
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}

		else if(strcmp("OnReleaseSuccess",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log->ERROR("onReleaseSuccess失败，失败原因%s",desc->value()));
				return -1;
			}
			curState = AfterCall;
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
			totalCall++;
		}





		if(strcmp("onAnswerSuccess",name->value()))
		{

		}
	}
}









CAgent::CAgent()
{
	sock = -1;
	sockState = 0;
	totalCall = 0;
	successCall = 0;
	curState = preState = SignOut;
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
	if(curState != SignOut)
	{
		log->ERROR("要inital的座席不是登出状态");
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
		log->ERROR("要signIn的座席不是在initial状态");
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






