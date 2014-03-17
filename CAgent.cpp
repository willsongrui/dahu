string getSession()
{
	string session = m_vccID + "-" + m_deviceID;
	return session;
}
string getHeader()
{
	char header[100];
	sprintf(header,"<header><sessionID>%s</sessionID><serialID></serialID><serviceID></serviceID><timeStamp>%s</timeStamp></header>",
		getSession(),m_timeStamp);
	return string(header);
}
int CAgent::sendMsg(string str)
{
	log()->LOG("将消息发送到队列中：%s",str);
	m_msgToSend.push_back(string(str));
	return 0;
}
int CAgent::sendMsgEx(string msg,char strName[])
{
	char strTrace[] = "off";
	char str[500];
	msg.replace(msg.find("<header></header>"),17,getHeader());
	sprintf(str,"%s %s %s %s",getCtiEventID(strName),strTrace,strName,msg);
	
	return sendMsg(str);
}
int CAgent::sendHeartBeat()
{
	char msg[100];
    sprintf(msg, "1000 off heartBeat <acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"response\" name=\"heartBeat\"/>"
        "</acpMessage>", m_sessionID);  

    return SendMsg(str);
}

int CAgent::report(int agentStatus,int phoneStatus)
{
	char msg[300];
    
    sprintf(msg,"<acpMessage ver=\"2.0.0\">"
        "<header></header>"
        "<body type=\"request\" name=\"AgentReport\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter agentStatus=\"%d\" phoneStatus =\"%d\"/>"
        "</body></acpMessage>",m_vccID,m_agentID,m_deviceID,m_agentStatus,m_phoneStatus);
    
    return sendMsgEx(msg,"AgentReport");    
}

int CAgent::setStatus(DetailState st,bool easy_mode = true)
{
	log()->LOG("座席状态试图从%d到%d",curState,st);
	if(easy_mode == true)
	{
		preState = curState;
		curState = st;
		if(st == SignIn)
		{
			return setIdle();
		}
		else if(st == AfterCall)
		{
			return setIdle();
		}
	}
	return 0;
}

int CAgent::handle_message(string msg)
{
	log()->LOG("收到消息 %s",msg.c_str());
	size_t pos = msg.find("<acpMessage");
	if(pos == string::npos)
	{
		log()->ERROR("座席收到的消息格式错误(没有<acp),具体消息为 %s",msg.c_str());
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
		log()->ERROR("消息解析错误");
		return -1;
	}

	if(strcmp("response",type->value()) == 0)
	{
		if(strcmp("Initial", name->value() == 0))
		{			
			xml_attribute<>* ip = parameter->first_attribute("ip");
			xml_attribute<>* port = parameter->first_attribute("port");
			setStatus(Initial);
			m_signIn_IP = string(ip->value());
			m_signIn_Port = htons(atoi(port->value()));
			return signIn(string(ip->value()),atoi(port->value()));
		}
		else
		{
			xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
			xml_node<>* timeStamp = header->first_node("timeStamp");
			if(!(sessionID && timeStamp))
			{
				m_timeStamp = timeStamp->value();
				m_sessionID = sessionID->value();
			}
			else
			{
				log()->ERROR("sessionID或timeStamp解析错误");
				//return -1;
			}
		}
		if(strcmp("SignIn", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("signIn失败，失败原因%s",desc->value());
				return -1;
			}
			xml_node<>* agentParam = body->first_node("agentParam");
			if(agentParam)
			{
				xml_attribute<>* idleStatus = agentParam->first_attribute("idleStatus");
				if(idleStatus)
				{	
					statusAfterCall = atoi(idleStatus->value());
					if((statusAfterCall == 3)||(statusAfterCall == 4))
					{
						xml_attribute<>* autoStatusInteval = agentParam->first_attribute("autoStatusInteval");
						if(autoStatusInteval)
						{
							inteval = atoi(autoStatusInteval->value());
						}
					}
				}
			}
			return setStatus(SignIn);
		}
		else if(strcmp("SignOut", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("signOut失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(SignOut);
		}
		else if(strcmp("SetBusy", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("setBusy失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(Busy);
			
		}
		else if(strcmp("SetIdle", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("setIdle失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(Idle);
		}
		else if(strcmp("AgentReconnect", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("AgentReconnect失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(SignIn);
		}

		else if(strcmp("ForceIdle", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("ForceIdle失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(Idle);
		}

		else if(strcmp("ForceBusy", name->value()) == 0)
		{
			
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("ForceBusy 失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(Busy);
		}

		else if(strcmp("ForceOut", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("forceOut失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(SignOut);
			
		}

		else if(strcmp("ReleaseCall", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("releaseCall失败，失败原因%s",desc->value());
				return -1;
			}
			return setStatus(AfterCall);
		}
	}
	else if(strcmp("event", type->value()))
	{
		xml_node<>* sessionID = root->first_node("header")->first_node("sessionID");
		xml_node<>* timeStamp = header->first_node("timeStamp");
		if(!(sessionID && timeStamp))
		{
			m_timeStamp = string(timeStamp->value());
			m_sessionID = string(sessionID->value());
		}
		else
		{
			log()->ERROR("sessionID或timeStamp解析错误");
			//return -1;
		}
		
		
		//CTI对处于后处理状态的座席置闲
		if(strcmp("OnSystemIdle", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnSystemIdle失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(Idle);
		}

		//当有其他相同座席注册时，被签出
		else if(strcmp("OnSignOuted", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnSignOuted失败，失败原因%s",desc->value()));
				return -1;
			}

			log()->ERROR("其他相同座席注册,被签出");
			return setStatus(SignOut);
		}
		//所有座席忙
		else if(strcmp("OnAgentAllBusy", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnAgentAllBusy失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(Busy);
			
		}

		//所有座席置闲
		if(strcmp("OnForceIdle",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceIdle失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(Idle);
		}

		//所有座席置忙
		if(strcmp("OnForceBusy",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceBusy失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(Busy);
		}

		//所有座席签出
		else if(strcmp("OnForceOut", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceOut失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(SignOut);
		}

		else if(strcmp("OnForceRelease", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceRelease失败，失败原因%s",desc->value()));
				return -1;
			}
			return setStatus(AfterCall);
		}

		else if(strcmp("OnAnswerRequest", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnAnswerRequest失败，失败原因%s",desc->value()));
				return -1;
			}
			xml_node<>* alertingDevice = body->first_node("alertingDevice");
			xml_node<>* autoStatusInteval = body->first_node("autoStatusInteval");
			if(!(alertingDevice && autoStatusInteval))
			{
				log()->ERROR("收到的OnAnswerRequest消息解析alertingDevice,autoStatusInteval失败");
			}
			else
			{
				log()->LOG("收到来自%s的呼叫请求",alertingDevice->value());
				xml_attribute<>* idleStatus = autoStatusInteval->first_attribute("idleStatus");
				if(!idleStatus)
				{
					log()->ERROR("收到的OnAnswerRequest消息解析idleStatus失败");
				}
				else
				{
					m_statusAfterCall = atoi(idleStatus->value());
					if(strcmp("0",idleStatus->value()) == 0)
					{
						log()->LOG("电话接通后所处的状态: 后续处理");
					}
					else if(strcmp("1",idleStatus->value()) == 0)
					{
						log()->LOG("电话接通后所处的状态: 自动空闲");
					}
					else if(strcmp("2",idleStatus->value()) == 0)
					{
						log()->LOG("电话接通后所处的状态: 自动置忙");
					}
					else if(strcmp("3",idleStatus->value()) == 0)
					{
						log()->LOG("电话接通后所处的状态: 时间间隔后自动空闲");
					}
					else if(strcmp("4",idleStatus->value()) == 0)
					{
						log()->LOG("电话接通后所处的状态: 时间间隔后自动置忙");
					}
					statusAfterCall = atoi(idleStatus->value());
				}
			}
			totalCall++;
		}
	
		else if(strcmp("OnAnswerSuccess",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnAnswerSuccess失败，失败原因%s",desc->value()));
				return -1;
			}
			return	setStatus(Calling);
			
		}

		else if(strcmp("OnAnserFailue",name->value()))
		{
			log()->ERROR("通话失败,收到answer_failue消息");
		}

		else if(strcmp("OnReleaseSuccess",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("onReleaseSuccess失败，失败原因%s",desc->value()));
				return -1;
			}
			succcessCall++;
			return setStatus(AfterCall);
		}
		return 0;
	}
}

CAgent::CAgent()
{
	m_timeStamp = m_sessionID = "";
	inteval = statusAfterCall = -1;
	sock = -1;
	sockState = 0;
	totalCall = 0;
	successCall = 0;
	curState = preState = SignOut;
	while(m_msgToSend.empty() == false)
	{
		m_msgToSend.pop();
	}
	while(msgRecieved.empty() == false)
	{
		msgRecieved.pop();
	}
}

int CAgent::initial()
{

	
	char msg[200];
	sprintf(msg,"<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"Initial\">"
        "<parameter vdcode=\"%s\"/>"
        "</body></acpMessage>",m_sessionID, m_vdcode);
	
	
	m_log()->LOG("座席发送Initial消息：%s",msg);
	setStatus(Try2Initial);
	return sendMsgEx(msg,"Initial");
}

int CAgent::signIn(string IP,int port)
{
	sockState = 0;
	
	
	char msg[300];
	sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SignIn\">"
 		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter deviceID=\"%s\" passwd=\"%s\" ready=\"%d\"/>"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID,m_deviceID,m_passwd,m_ready);
	m_log()->LOG("座席发送signin消息：%s",msg);
    
    setStatus(Try2SignIn);
	return sendMsgEx(msg,"SignIn");

}


int CAgent::signOut(string IP,int port)
{

	char msg[300];
	sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SignOut\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter />"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID);
	
	m_log()->LOG("座席发送signOut消息：%s",msg);
	setStatus(Try2SignOut);
	return sendMsgEx(msg,"Signout");
}

int CAgent::setIdle()
{
	char msg[300];

    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SetIdle\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter />"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID);
   	
	m_log()->LOG("座席发送setIdle消息：%s",msg);
	setStatus(Try2SetIdle);
	return sendMsgEx(msg,"SetIdle");
}

int CAgent::setBusy()
{
	char msg[300];
    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SetBusy\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter />"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID);
   	
	m_log()->LOG("座席发送setBusy消息：%s",msg);
	setStatus(Try2SetBusy);
	return sendMsgEx(msg,"SetBusy");
}

int CAgent::forceBusy()
{
	char msg[300];
    
    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"ForceBusy\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter destAgentID=\"%s\"/>"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID,m_destAgentID);
    
      
   	
	m_log()->LOG("座席发送forceBusy消息：%s",msg);
	setStatus(Try2ForceBusy);
	return sendMsgEx(msg,"ForceBusy");
}

int CAgent::forceIdle()
{
	char msg[300];
    
    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"ForceIdle\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter destAgentID=\"%s\"/>"
        "</body></acpMessage>",m_sessionID ,m_vccID,m_agentID,m_deviceID,m_destAgentID);
    
      
   	
	m_log()->LOG("座席发送forceIdle消息：%s",msg);
	setStatus(Try2ForceIdle);
	return sendMsgEx(msg,"ForceIdle");
}


int CAgent::forceOut()
{
	char msg[300];
    
    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"ForceOut\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter destAgentID=\"%s\"/>"
        "</body></acpMessage>", m_sessionID,m_vccID,m_agentID,m_deviceID,m_destAgentID);
    
	m_log()->LOG("座席发送forceOut消息：%s",msg);
	setStatus(Try2ForceOut);
	return sendMsgEx(msg,"ForceOut");
}

int CAgent::releaseCall()
{
	char msg[300];
    
    sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"ReleaseCall\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter />"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID);    
	m_log()->LOG("座席发送releaseCall消息：%s",msg);
	setStatus(Try2ReleaseCall);
	return sendMsgEx(msg,"ReleaseCall");
}





