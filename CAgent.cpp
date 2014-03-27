#include <queue>
#include <stdio.h>
#include <string>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "simu_def.h"
#include "simulation.h"
#include <sys/types.h>
#include <sys/socket.h>



using namespace rapidxml;
using namespace std;
#define RELEASE_AND_EXIT delete header, body, \
	timeStamp, sessionID, type, name;\
	return -1;
#define RELEASE_AND_EXIT_0 delete header, body, \
	timeStamp, sessionID, type, name;\
	return 0; 

std::map <std::string,EventType_t> CAgent::eventTypeMap;
extern CCenter center;
//queue<int> socket_not_in_epoll;


/*
int CAgent::send_message()
{
	if(m_connected == false)
		log()->ERROR("座席socket还没准备好");
	string msgToSend;
	while(m_msgToSend.empty()==false)
	{
		msgToSend = m_msgToSend.front();
		if(send(sock,msgToSend.c_str(),sizeof(msgToSend),0) < 0)
		{
			log()->ERROR("试图发送msg时失败，msg为：%s，错误原因为：%s", msgToSend, strerror(errno));
		}
		m_msgToSend.pop();

	}
	return 0;	
}
*/

int CAgent::send_message(int sockFd)
{
	int type = find_sock_type(sockFd);
	string msgToSend;
	if(type == 0)
	{
		while(m_initial_msgToSend.empty()==false)
		{
			msgToSend = m_initial_msgToSend.front();
			m_initial_msgToSend.pop();
			if(send(sockFd, msgToSend.c_str(), sizeof(msgToSend), 0)<0)
			{
				log()->ERROR("试图发送Initial msg时失败，msg为：%s，错误原因为：%s", msgToSend.c_str(), strerror(errno));
				return -1;
			}
		}
	}
	else if(type == 1)
	{
		while(m_signIn_msgToSend.empty()==false)
		{
			msgToSend = m_signIn_msgToSend.front();
			m_signIn_msgToSend.pop();
			if(send(sockFd, msgToSend.c_str(), sizeof(msgToSend), 0)<0)
			{
				log()->ERROR("试图发送Initial msg时失败，msg为：%s，错误原因为：%s", msgToSend.c_str(), strerror(errno));
				return -1;
			}
		}

	}
	else
	{
		log()->ERROR("传入socket与当前座席的Initial和signIn都不匹配");
		return -1;
	}
}



string CAgent::getHeader()
{
	char header[100];
	sprintf(header,"<header><sessionID>%s</sessionID><serialID></serialID><serviceID></serviceID><timeStamp>%s</timeStamp></header>",
		m_sessionID, m_timeStamp);
	return string(header);
}
int CAgent::sendMsg(string& str, int type)
{
	log()->LOG("将消息发送到队列中：%s",str.c_str());
	if(type==1)
		m_signIn_msgToSend.push(str);
	else
		m_initial_msgToSend.push(str);
	return 0;
}
int CAgent::sendMsgEx(string& msg,const char* strName)
{
	int type = 1;
	if(strcmp(strName, "initial")==0)
		type = 0;
	char strTrace[] = "on";
	char str[500];
	msg.replace(msg.find("<header></header>"),17,getHeader());
	sprintf(str,"%s %s %s %s","1000", strTrace,strName,msg.c_str());
	string temp = string(str);
	return sendMsg(temp, type);
}
int CAgent::sendHeartBeat()
{
	char msg[100];
    sprintf(msg, "1000 off heartBeat <acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"response\" name=\"heartBeat\"/>"
        "</acpMessage>", m_sessionID);  
    string temp = string(msg);
    return sendMsg(temp, 1);
}

int CAgent::agentReport()
{
	char msg[300];
    
    sprintf(msg,"<acpMessage ver=\"2.0.0\">"
        "<header></header>"
        "<body type=\"request\" name=\"AgentReport\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter agentStatus=\"%d\" phoneStatus =\"%d\"/>"
        "</body></acpMessage>",m_vccID,m_agentID,m_deviceID,m_agentStatus,m_phoneStatus);
    string temp = string(msg);
    return sendMsgEx(temp, "AgentReport");    
}

int CAgent::setAgentStatus(DetailState_t st,bool easy_mode = true)
{
	log()->LOG("座席状态试图从%d到%d",m_curState,st);
	if(easy_mode == true)
	{
		m_preState = m_curState;
		m_curState = st;
	
	}
	return 0;
}

string find_cmd(const string& msg, int end)
{
	int pos;
	pos = msg.find("1000 off");
	string cmd = "NONEXIST";
	if(pos != string::npos)
	{
		cmd = msg.substr(pos + 8, end - pos - 8);
	}
	else
	{
		pos = msg.find("1000 on");
		if(pos != string::npos)
		{
			cmd = msg.substr(pos + 7, end - pos - 7);
		}
	}
	int left = 0;
	int right = cmd.length();
	for(int i=0; i<cmd.length(); i++)
	{
		if(cmd[i] == ' ')
		{
			left += 1;
		}
		else
			break;

	}
	for(int i=cmd.length()-1;i >= 0; i--)
	{
		if(cmd[i] == ' ')
		{
			right -= 1;
		}
		else
			break;

	}
	return cmd.substr(left,right-left);
}	


int CAgent::handle_message(string& msg, int sockFd, bool quick)
{

	int type = find_sock_type(sockFd);

	log()->LOG("收到消息 %s",msg.c_str());
	size_t pos = msg.find("<acpMessage");
	if(pos == string::npos)
	{
		log()->ERROR("座席收到的消息格式错误(没有<acp),具体消息为 %s",msg.c_str());
		return -1;
	}
	if(quick == true)
	{
		string cmd = find_cmd(msg, pos);
		if(cmd != "NONEXIST")
		{
			log()->ERROR("收到消息没有命令码");
		}
		else
		{
			/*
			if(allowed_cmd.find(cmd) == allowed_cmd.end())
			{
				log()->LOG("该消息的命令码是%s,被忽略");
				return 0;
			}
			*/
		}
	}
	string xml_msg = msg.substr(pos);
	if(msgParse(xml_msg)<0)
	{
		log()->ERROR("msgParse 错误");
		return -1;
	}
	return handle_msg();
}
int CAgent::find_sock_type(int sockFd)
{
	if(sockFd == m_initial_sock)
		return 0;
	if(sockFd == m_signIn_sock)
		return 1;
	return -1;
}

//根据成员变量 m_acpEvent
int CAgent::handle_msg()
{
	ACPEvent_t* msg = (ACPEvent_t*)&m_acpEvent;
	if(msg == NULL)
		return -1;
    switch (msg->eventHeader.eventType) 
    {
	case ACP_Initial_CONF:
		{
			if(msg->event.acpConfirmation.u.initialConf.code == 0)
			{
				log()->LOG("成功收到initial Response消息");
				m_signIn_IP = msg->event.acpConfirmation.u.initialConf.ip;
				m_signIn_Port = msg->event.acpConfirmation.u.initialConf.port;				
				m_lMsgReceived = MSG_SUCCESS;
				//添加连接
				//int sock = create_sock_non_blocking(m_signIn_IP, m_signIn_Port);
				int sock = create_connection_to_cti(m_signIn_IP, m_signIn_Port,this);
				if(sock < 0)
				{
					log()->ERROR("试图创建signin socket时失败");
					return -1;
				}
				add_to_epoll(sock);
				m_signIn_sock = sock;

				//close initial sock???
				close(m_initial_sock);
				return signIn();
			}
			else
			{
				log()->ERROR("Initial失败,错误码是%d,描述为",msg->event.acpConfirmation.u.initialConf.code,msg->event.acpConfirmation.u.initialConf.desc);
				m_lMsgReceived = MSG_FAILURE;
				return -1;
			}
			break;
		}
	case ACP_SignIn_CONF:
			if(msg->event.acpConfirmation.u.signOutcConf.parameter.cause.code == 0)
			{
				log()->LOG("成功收到signIn Response消息");
				int idleStatus = msg->event.acpConfirmation.u.signOutcConf.agentParam.idleStatus;
				if(idleStatus == 0)
				{
					return setIdle();
				}
				else
				{
					log()->LOG("座席的后续处理状态是%d,系统不自动发送setIdle请求",idleStatus);
					if(idleStatus == 1)
					{
						setAgentStatus(AGENT_IDLE);
					}
					else if(idleStatus == 2)
					{
						setAgentStatus(AGENT_BUSY);
					}		
					else if(idleStatus == 3)
					{
						setAgentStatus(AGENT_IDLE);
					}
				}
				/*acpSetCtiEventEx(msg->event.acpConfirmation.u.signOutcConf.agentParam.ctiEvent,msg->eventHeader.timeStamp,
					msg->event.acpConfirmation.u.signOutcConf.parameter.agent.vccID,msg->event.acpConfirmation.u.signOutcConf.parameter.agent.agentID,
					msg->event.acpConfirmation.u.signOutcConf.parameter.agent.deviceID);
				setAgentStatusEx(msg->event.acpConfirmation.u.signOutcConf.parameter.agent.agentID,emSignIn,1);
				SetAgentReportStatus(msg->event.acpConfirmation.u.signOutcConf.parameter.agent.agentID,
					msg->event.acpConfirmation.u.signOutcConf.parameter.agent.agentStatus,
					msg->event.acpConfirmation.u.signOutcConf.parameter.agent.phoneStatus);*/
			}
			else
			{
				log()->ERROR("signIn失败,错误码为%d, 描述为%s",msg->event.acpConfirmation.u.signOutcConf.parameter.cause.code, msg->event.acpConfirmation.u.signOutcConf.parameter.cause.desc);
				//setAgentStatusEx(msg->event.acpConfirmation.u.signOutcConf.parameter.agent.agentID,emSignIn,0);
				return -1;
			}
			break;
	case ACP_SignOut_CONF:
		{
			if(msg->event.acpConfirmation.u.generalConf.cause.code == 0)
			{
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSignOut,1);
				setAgentStatus(AGENT_SIGNOUT);
			}
			else
			{
				log()->ERROR("signOut失败，错误码为%d, 描述为%s", msg->event.acpConfirmation.u.generalConf.cause.code, msg->event.acpConfirmation.u.generalConf.cause.desc);
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSignOut,0);
				return -1;
			}
			break;
		}
	case ACP_SetBusy_CONF:
		{
			if(msg->event.acpConfirmation.u.generalConf.cause.code == 0)
			{
				setAgentStatus(AGENT_BUSY);
				/*setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSetBusy,1);
				SetAgentReportStatus(msg->event.acpConfirmation.u.generalConf.agent.agentID,
				msg->event.acpConfirmation.u.generalConf.agent.agentStatus,
				msg->event.acpConfirmation.u.generalConf.agent.phoneStatus);*/
			}
			else
			{
				log()->ERROR("setBusy失败，错误码为%d，描述为%s", msg->event.acpConfirmation.u.generalConf.cause.code, msg->event.acpConfirmation.u.generalConf.cause.desc);
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSetBusy,1);
				return -1;
			}
			break;
		}
 	case ACP_SetIdle_CONF:
		{
			if(msg->event.acpConfirmation.u.generalConf.cause.code == 0)
			{
				setAgentStatus(AGENT_IDLE);

				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSetIdle,1);
			}
			else
			{
				log()->ERROR("setIdle失败，错误码为%d，描述为%s", msg->event.acpConfirmation.u.generalConf.cause.code, msg->event.acpConfirmation.u.generalConf.cause.desc);
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emSetIdle,0);
				return -1;
			}
			/*
			SetAgentReportStatus(msg->event.acpConfirmation.u.generalConf.agent.agentID,
				msg->event.acpConfirmation.u.generalConf.agent.agentStatus,
				msg->event.acpConfirmation.u.generalConf.agent.phoneStatus);*/
			break;
		}
	case ACP_OnAnswerRequest:
		{
			if(strcmp(msg->event.acpEventReport.u.generalEventReport.agent.deviceID,msg->event.acpEventReport.u.answerRequestEventReport.alertingDevice)==0)
			{
				log()->LOG("OnAnswerRequest 成功");
				//setAgentStatus(AGENT_ONANSWER);
			}
			else
			{
				log()->ERROR("OnAnswerRequest 失败");
				return -1;
			}
			
		}
		break;
	case ACP_ReleaseCall_CONF:
		{
			if(msg->event.acpConfirmation.u.generalConf.cause.code == 0)
			{
				log()->LOG("ReleaseCall成功");
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emReleaseCall,1);
			}
			else
			{
				log()->ERROR("ReleaseCall失败");
				return -1;
				//setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emReleaseCall,0);
			}
			/*
			SetAgentReportStatus(msg->event.acpConfirmation.u.generalConf.agent.agentID,
				msg->event.acpConfirmation.u.generalConf.agent.agentStatus,
				msg->event.acpConfirmation.u.generalConf.agent.phoneStatus);*/
		break;
		}
	/*
	case ACP_CallOutside_CONF:
		{
			if(msg->event.acpConfirmation.u.generalConf.cause.code == 0)
			{
				setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emCallOutSide,1);
			}
			else
			{
				setAgentStatusEx(msg->event.acpConfirmation.u.generalConf.agent.agentID,emCallOutSide,0,msg->event.acpConfirmation.u.generalConf.agent.deviceID);
			}
			SetAgentReportStatus(msg->event.acpConfirmation.u.generalConf.agent.agentID,
				msg->event.acpConfirmation.u.generalConf.agent.agentStatus,
				msg->event.acpConfirmation.u.generalConf.agent.phoneStatus);
			break;
		}
		break;
		*/
	case ACP_OnForceRelease:
		{
			/*setAgentStatusEx(msg->event.acpEventReport.u.generalEventReport.agent.agentID,emForceRelease,1,msg->event.acpEventReport.u.releaseEventReport.releaseDevice);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.generalEventReport.agent.agentID,
				msg->event.acpEventReport.u.generalEventReport.agent.agentStatus,
				msg->event.acpEventReport.u.generalEventReport.agent.phoneStatus);
			if( msg->event.acpEventReport.u.releaseEventReport.callInfo.count> 0 )
			{
				delete [] msg->event.acpEventReport.u.releaseEventReport.callInfo.device;//.callInfo.callinfo.device;
				msg->event.acpEventReport.u.releaseEventReport.callInfo.count = 0;
				msg->event.acpEventReport.u.releaseEventReport.callInfo.device = NULL;
			}*/
			log()->LOG("forceRelease");
		}
		break;
	case ACP_OnReleaseSuccess:
		{
			//20120727
			if(strcmp(msg->event.acpEventReport.u.releaseEventReport.releaseDevice,msg->event.acpEventReport.u.generalEventReport.agent.deviceID)==0)
			{
				setAgentStatus(AGENT_RELEASE_SUCCESS);
				msg->event.acpEventReport.u.releaseEventReport.callInfo.device = NULL;
				log()->LOG("Release Success");
				
			}
			else
			{
				log()->ERROR("Release 失败");
				return -1;
			}
		}
		break;
	/*
	case ACP_OnCallOutsideSuccTalk:
		{
			setAgentStatusEx(msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,emCallOutSideEvent,1);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentStatus,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.phoneStatus);
		}
		break;
	case ACP_OnCallOutsideSuccess:
		{
			setAgentStatusEx(msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,emCallOutSideEvent,0);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentStatus,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.phoneStatus);
		}
		break;
	case ACP_OnCallOutsideFailure:
		{
			setAgentStatusEx(msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,emCallOutSideEvent,-1);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentStatus,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.phoneStatus);
		}
		break;
		*/
	case ACP_OnAnswerFailure:
		{
			setAgentStatus(AGENT_IDLE);
			m_failure_call_num += 1;
			/*
			setAgentStatusEx(msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,emAnswer,0);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentStatus,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.phoneStatus);*/
		}
		break;
	case ACP_OnAnswerSuccess:
		{
			setAgentStatus(AGENT_ANSWERING);
			m_success_call_num += 1;
			/*
			setAgentStatusEx(msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,emAnswer,1);
			SetAgentReportStatus(
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentID,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.agentStatus,
				msg->event.acpEventReport.u.callinfoEventReport.parameter.agent.phoneStatus);*/
		}
		break;
	case ACP_AgentReport_CONF:
		{
		
		}
		break;
	}
	return 0;

}






int CAgent::BuildGeneralConf(ACPGeneralConfEvent_t &generalConf,xml_node<>* body)
{
	//ASSERT(body!=NULL);
	if(body == NULL)
		return -1;
	int buildInfo = BuildAgentInfo(generalConf.agent,body);
	int causeInfo = BuildCauseInfo(generalConf.cause,body);
	if((buildInfo<0)||(causeInfo<0))
		return -1;

}
int CAgent::BuildAgentInfo(Agent_t &agentInfo,xml_node<>* body)
{
    //ASSERT(body != NULL);
	if(body == NULL)
		return -1;
    xml_node<>* agent = body->first_node("agent");
    if(agent == NULL)
    {
    	log()->ERROR("BuildAgentInfo 错误");
    	return -1;
    }
    xml_attribute<>* vccID = agent->first_attribute("vccID");
    xml_attribute<>* agentID = agent->first_attribute("agentID");
    xml_attribute<>* deviceID = agent->first_attribute("deviceID");
    	xml_attribute<>* phoneStatus = agent->first_attribute("phoneStatus");
	xml_attribute<>* agentStatus = agent->first_attribute("agentStatus");
    if(!(vccID&&agentID&&deviceID&&phoneStatus&&agentStatus))
	{
    	log()->ERROR("BuildAgentInfo 错误");
    	delete vccID, agentID, deviceID, phoneStatus, agentStatus;
    	return -1;
    }
	strcpy(agentInfo.vccID, vccID->value());
	strcpy(agentInfo.agentID, agentID->value());
	strcpy(agentInfo.deviceID, deviceID->value());
	
	strcpy(m_vccID, vccID->value());
	strcpy(m_agentID, agentID->value());
	strcpy(m_deviceID, deviceID->value());
	m_agentStatus = (AgentState_t)atoi(agentStatus->value());
	m_phoneStatus = (PhoneState_t)atoi(phoneStatus->value());;
	
	agentInfo.master = 0;
	xml_attribute<>* master = agent->first_attribute("master");
	if(master)
		agentInfo.master = atoi(master->value());
	
	delete vccID, agentID, deviceID, master, agentStatus, phoneStatus;
	return 0;
}

int CAgent::BuildCauseInfo(Cause_t &causeInfo,xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
    xml_node<>* cause  = body->first_node("cause");
	if(!cause)
	{
		log()->ERROR("BuiilCauseInfo 错误");
		return -1;
	}
	xml_attribute<>* code = cause->first_attribute("code");
	xml_attribute<>* desc = cause->first_attribute("desc");
	if(!(code&&desc))
	{
		log()->ERROR("BuiilCauseInfo 错误");
		return -1;
	}	
	if(strcmp(" ", code->value())==0)
	{
		causeInfo.code = atoi(code->value());
		strcpy(causeInfo.desc, desc->value());

	}
	else
	{
		causeInfo.code = 0;
		strcpy(causeInfo.desc, "");
	}
	if(causeInfo.code!=0)
	{
		log()->ERROR("收到错误消息，cause code为%d，失败描述为%s", causeInfo.code, causeInfo.desc);
	}

	delete code,desc,cause;
	return 0;
}
int CAgent::BuildIntialConf(ACPInitialConfEvent_t &initialConf,xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
    xml_node<>* parameter  = body->first_node();
    if(!parameter)
    {
    	log()->ERROR("BuildIntialConf失败");
    	return -1;
    }
    xml_attribute<>* code = parameter->first_attribute("code");
    xml_attribute<>* desc = parameter->first_attribute("desc");
    xml_attribute<>* ip = parameter->first_attribute("ip");
    xml_attribute<>* port = parameter->first_attribute("port");
    if(!(code&&desc&&ip&&port))
    {
    	log()->ERROR("BuildIntialConf失败");
    	return -1;

    }
	initialConf.code = atoi(code->value());
	strcpy(initialConf.desc, desc->value());
	strcpy(initialConf.ip, ip->value());
	initialConf.port = atol(port->value());
	
	m_signIn_IP = string(ip->value());
	//strcpy(m_signIn_IP, ip->value());
	m_signIn_Port = atoi(port->value());

	delete code,desc,ip,port,parameter;
	return 0;
}
int CAgent::BuildCallinfo(Callinfo_t &callInfo, xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
// 	BuildAgentInfo(callinfoEventReport.parameter.agent,body);
//	BuildCauseInfo(callinfoEventReport.parameter.cause,body);
	Agent_t agentInfo;
	BuildAgentInfo(agentInfo,body);
	if(agentInfo.phoneStatus == CS_IDLE)
		return 0;
	xml_node<>* HCallinfo = body->first_node("callinfo");
    
	if(HCallinfo == NULL)
	{
		log()->ERROR("BuildCallInfo失败");
		return -1;
	}
	xml_attribute<>* calledDevice = HCallinfo->first_attribute("calledDevice");
	if(calledDevice!=NULL)
	{
		strcpy(callInfo.calledDevice,calledDevice->value());
		strcpy(m_calledDevice, callInfo.calledDevice);
	}

	xml_attribute<>* origCalledDevice = HCallinfo->first_attribute("origCalledDevice");
	if(origCalledDevice!=NULL)
	{
		strcpy(callInfo.orgCalledDevice, origCalledDevice->value());
		strcpy(m_orgCalledDevice, callInfo.orgCalledDevice);
	}


	xml_attribute<>* CallingDevice = HCallinfo->first_attribute("CallingDevice");
	if(CallingDevice!=NULL)
	{
		strcpy(callInfo.callingDevice,CallingDevice->value());
		strcpy(m_callingDevice, callInfo.callingDevice);
	}

	xml_attribute<>* orgCallingDevice = HCallinfo->first_attribute("origCallingDevice");
	if(orgCallingDevice!=NULL)
	{
		strcpy(callInfo.orgCallingDevice,orgCallingDevice->value());
		strcpy(m_orgCallingDevice, callInfo.orgCallingDevice);
	}

	xml_attribute<>* callData = HCallinfo->first_attribute("callData");
	if(callData!=NULL)
		strcpy(callInfo.callData, callData->value());

	xml_attribute<>* count = HCallinfo->first_attribute("count");
	if(count!=NULL)
		callInfo.count = atoi(count->value());
	/*
	if(callInfo.count>0)
		callInfo.device = new Device_t[count];
	strcpy(callInfo.device[0].deviceID,agentInfo.deviceID);
	callInfo.device[0].legID = 6;
	callInfo.device[0].phoneStatus = agentInfo.phoneStatus;
	
	for (int i=1; i<count; ++i)
	{
		TXMLHandle hDevice = sXmlGetChild(HCallinfo, i-1);
		strcpy(callInfo.device[i].deviceID,sXmlGetAttrValue(hDevice, "deviceID"));
		callInfo.device[i].legID = atoi(sXmlGetAttrValue(hDevice, "legID"));
		callInfo.device[i].phoneStatus = (PhoneState_t)atoi(sXmlGetAttrValue(hDevice, "phoneStatus"));
	}*/
	delete HCallinfo, calledDevice, orgCallingDevice, origCalledDevice, orgCallingDevice, callData, count;
	return 0;
}

int CAgent::BuildGeneralEventReport(ACPEventReportEvent_t &generalEventReport,xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
 	int agentRet = BuildAgentInfo(generalEventReport.agent,body);
	int causeRet = BuildCauseInfo(generalEventReport.cause,body);
	/*
   TXMLHandle hParameter  = xmlGetElememt(body, "parameter");
    const char* code = sXmlGetAttrValue(hParameter, "code");
	generalEventReport.cause.code = atoi(code);
	strcpy(generalEventReport.cause.desc,sXmlGetAttrValue(hParameter, "desc"));
	generalEventReport.agent.agentStatus = (AgentState_t)atoi(sXmlGetAttrValue(hParameter, "agentStatus"));
	generalEventReport.agent.phoneStatus = (PhoneState_t)atoi(sXmlGetAttrValue(hParameter, "phoneStatus"));
	generalEventReport.agent.master = atoi(sXmlGetAttrValue(hParameter, "master"));
	*/
	if((agentRet<0) || (causeRet<0))
	{
		log()->ERROR("BuildGeneralEventReport错误");
		return -1;
	}
	return 0;
}


int CAgent::BuildAnswerRequestEventReport(ACPEvent_t &msg,xml_node<>* body)
{
	int ret = BuildGeneralEventReport(msg.event.acpEventReport.u.answerRequestEventReport.parameter,body);

	
    xml_node<>* hAlertingDevice = body->first_node("alertingDevice");
	if(hAlertingDevice!=NULL)
	{
		strcpy(msg.event.acpEventReport.u.answerRequestEventReport.alertingDevice,hAlertingDevice->value());
	}
	int CallinfoRet = BuildCallinfo(msg.event.acpEventReport.u.answerRequestEventReport.callInfo,body);	
	if(ret < 0 || CallinfoRet < 0)
	{
		log()->ERROR("BuildAnswerRequestEventReport 错误");
		return -1;
	}
	return 0;
}

int CAgent::BuildHangupCallInfo(ACPHangupCallEvent_t &hangupCallEventReport,xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
 	int agentRet = BuildAgentInfo(hangupCallEventReport.parameter.agent,body);
	int causeRet = BuildCauseInfo(hangupCallEventReport.parameter.cause,body);
    xml_node<>* hCallInfo  = body->first_node("callInfo");
	if(hCallInfo!=NULL)
	{
		strcpy(hangupCallEventReport.callingNo, hCallInfo->value());
		strcpy(hangupCallEventReport.calledNo, hCallInfo->value());
	}
	if(agentRet<0 || causeRet < 0)
	{
		log()->ERROR("BuildHangupCallInfo 错误");
		delete hCallInfo;
		return -1;
	}
	delete hCallInfo;
	return 0;
}

int CAgent::BuildReleaseEventReport(ACPEvent_t &msg,xml_node<>* body)
{
	//ASSERT(body != NULL);
	if(body == NULL)
    	return -1;
	BuildGeneralEventReport(msg.event.acpEventReport.u.releaseEventReport.parameter,body);

    xml_node<>* hRelease   = body->first_node("releaseDevice");
	xml_attribute<>* releaseLegID = NULL;
	if(hRelease==NULL)
	{
		log()->ERROR("BuildReleaseEventReport 错误");
		return -1;
	}

	if(hRelease)
	{
		strcpy(msg.event.acpEventReport.u.releaseEventReport.releaseDevice,hRelease->value());
		releaseLegID = hRelease->first_attribute("legID");
		if(releaseLegID)
			msg.event.acpEventReport.u.releaseEventReport.releaseLegID = atoi(releaseLegID->value());

	}
	delete releaseLegID, hRelease;
	return 0;
}



int CAgent::BuildRecordInfo(ACPRecordInfoEvent_t &recordInfoEventReport, xml_node<>* body)
{
    //ASSERT(body != NULL);
    if(body == NULL)
    	return -1;
 	int agentInfo = BuildAgentInfo(recordInfoEventReport.parameter.agent,body);
	int causeInfo = BuildCauseInfo(recordInfoEventReport.parameter.cause,body);
    /*xml_node<>* hRecordInfo  = xmlGetElememt(body, "fileName");
	if(hRecordInfo)
	{
		const char* fileName = sXmlGetValue(hRecordInfo);
		if(fileName)
			strcpy(recordInfoEventReport.fileName,fileName);
	}*/
	if(agentInfo<0 || causeInfo <0)
	{
		log()->ERROR("BuildRecordInfo 错误");
		return -1;
	}
	return 0;
}
/*
void CAgent::get_agent_phone_status(xml_node<>* body)
{
	if(body==NULL)
		return;
	xml_node<>* agent = body->first_node("parameter");
	if(agent==NULL)
		return;
	xml_attribute<>* phoneStatus = first_attribute("phoneStatus");
	xml_attribute<>* agentStatus = first_attribute("agentStatus");
	if(phoneStatus!=NULL)
		this->phoneStatus = atoi(phoneStatus);
	if(agentStatus!=NULL)
		this->agentStatus = atoi(agentStatus);
	delete agent, phoneStatus, agentStatus;
}
*/
int CAgent::BuildCallinfoEventReport(ACPEvent_t &msg,xml_node<>* body)
{
	//ASSERT(body);
	if(body == NULL)
    	return -1;
	int general = BuildGeneralEventReport(msg.event.acpEventReport.u.callinfoEventReport.parameter,body);
	int call = BuildCallinfo(msg.event.acpEventReport.u.callinfoEventReport.callInfo,body);	
	if(general<0||call<0)
	{
		log()->ERROR("BuildCallinfoEventReport错误");
		return -1;
	}
	return 0;
}


int CAgent::msgParse(string& msg)
{
	xml_document<> doc;
	try
	{
		doc.parse<0>(msg.c_str());
	}
	catch (parse_error e)
	{
		log()->ERROR("解析座席消息错误,%s", msg.c_str());
		return -1;
	}
	xml_node<>* root = doc.first_node("acpMessage");
	xml_node<>* header = NULL, *body = NULL;
	xml_node<>* timeStamp = NULL, *sessionID = NULL, *type = NULL, *name = NULL;
	if(!root)
		RELEASE_AND_EXIT
	else
	{
		header = root->first_node("header");
		body = root->first_node("body");
		if(!(header&&body))
			RELEASE_AND_EXIT
		else
		{
			timeStamp = header->first_node("timeStamp");
			sessionID = header->first_node("sessionID");
			type = body->first_attribute("type");
			name = body->first_attribute("name");
			if(!(timeStamp&&sessionID&&type&&name))
				RELEASE_AND_EXIT
			else
			{
				strncpy(m_acpEvent.eventHeader.timeStamp, timeStamp->value(), sizeof(timeStamp->value()));
				strncpy(m_acpEvent.eventHeader.sessionID, sessionID->value(), sizeof(sessionID->value()));
				strcpy(m_sessionID, sessionID->value());
				
				if(strcmp(type->value(), "request")==0)
				{
					m_acpEvent.eventClass = EC_REQUEST;
				}
				else if(strcmp(type->value(), "response")==0)
				{
					m_acpEvent.eventClass = EC_CONFIRMATION;
				}
				else if(strcmp(type->value(), "event")==0)
				{
					m_acpEvent.eventClass = EC_UNSOLICITED;
				}
				else
				{
					log()->ERROR("type字段被解析成%s，我们不认识", type->value());
					RELEASE_AND_EXIT
				}
				map <std::string,EventType_t>::iterator it;
				it = CAgent::eventTypeMap.find(name->value());

				if(it != CAgent::eventTypeMap.end())
				{
					m_acpEvent.eventHeader.eventType = it->second;
				}
				else
					m_acpEvent.eventHeader.eventType = ACP_UNKNOWN;
				
				
				//get_agent_phone_status(body);
				


				switch (m_acpEvent.eventHeader.eventType)
				{
					case ACP_Initial_CONF:
						BuildIntialConf(m_acpEvent.event.acpConfirmation.u.initialConf,body);
						break;
					case ACP_SignIn_CONF:
						BuildGeneralConf(msg->event.acpConfirmation.u.signOutcConf.parameter,body);
						xml_node<>* hAgentParam = body->first_node("agentParam");
						
						if(hAgentParam)
						{
							msg->event.acpConfirmation.u.signOutcConf.agentParam.idleStatus = 0;
							xml_attribute<>* idleStatus = hAgentParam->first_attribute("idleStatus");
				
							if(idleStatus)
							{
								msg->event.acpConfirmation.u.signOutcConf.agentParam.idleStatus = atoi(idleStatus->value());
								m_idleStatus = atoi(idleStatus->value());
							}
							//strcpy(msg->event.acpConfirmation.u.signOutcConf.agentParam.groupID,sXmlGetAttrValue(hAgentParam,"locked"));
							xml_attribute<>* agentType = hAgentParam->first_attribute("agentType");
							if(agentType)
							{
								m_agentType = atoi(agentType->value());
								msg->event.acpConfirmation.u.signOutcConf.agentParam.agentType = atoi(agentType->value());
							}
							xml_attribute<>* locked = hAgentParam->first_attribute("locked");
							if(locked)
							{
								msg->event.acpConfirmation.u.signOutcConf.agentParam.locked = atoi(locked->value());
								m_locked = atoi(locked->value());
							}
							xml_attribute<>* allTimeRecord = hAgentParam->first_attribute("allTimeRecord");
							if(allTimeRecord)
							{
								msg->event.acpConfirmation.u.signOutcConf.agentParam.allTimeRecord = atoi(allTimeRecord->value());
								m_allTimeRecord = atoi(allTimeRecord->value());
							}
							xml_attribute<>* deviceType = hAgentParam->first_attribute("deviceType");
							if(deviceType)
							{
								msg->event.acpConfirmation.u.signOutcConf.agentParam.deviceType = atoi(deviceType->value());
								m_deviceType = atoi(deviceType->value());
							}
							xml_attribute<>* ctiEvent = hAgentParam->first_attribute("ctiEvent");
							if(ctiEvent)
								strcpy(msg->event.acpConfirmation.u.signOutcConf.agentParam.ctiEvent,ctiEvent->value());
							else
								strcpy(msg->event.acpConfirmation.u.signOutcConf.agentParam.ctiEvent,"");
							delete idleStatus, agentType, locked, allTimeRecord, deviceType, ctiEvent;
						}
						break;
					
						
					case ACP_SignOut_CONF:
					case ACP_SetBusy_CONF:
					case ACP_SetIdle_CONF:
					case ACP_AgentReport_CONF:
					case ACP_SetAgentAutoEnterIdle_CONF:
					case ACP_AgentEnterIdle_CONF:
					case ACP_AgentEnterWrapUp_CONF:
					case ACP_ForceIdle_CONF:
					case ACP_ForceBusy_CONF:
					case ACP_ForceOut_CONF:
					case ACP_ReleaseCall_CONF:
					case ACP_ReleaseCallByCallID_CONF:
					case ACP_SendDTMF_CONF:
					case ACP_CallInside_CONF:
					case ACP_Consultation_CONF:
					case ACP_CallOutside_CONF:
					case ACP_SingleStepTransfer_CONF:
					case ACP_Transfer_CONF:
					case ACP_SetTransfer_CONF:
					case ACP_Hold_CONF:
					case ACP_RetrieveHold_CONF:
					case ACP_Conference_CONF:
					case ACP_Alternate_CONF:
					case ACP_DoReplyMultimeAndHangUpCall_CONF:
					case ACP_Listen_CONF:
					case ACP_Intercept_CONF:
					case ACP_Insert_CONF:
					case ACP_StopInsert_CONF:
					case ACP_BeginRecord_CONF:
					case ACP_StopRecord_CONF:
					case ACP_Bridge_CONF:
					case ACP_SetCTIParam_CONF:
					case ACP_SetCallData_CONF:
						BuildGeneralConf(msg->event.acpConfirmation.u.generalConf,body);
						break;
					case ACP_QueryAgentStatus_CONF:
						BuildGeneralConf(msg->event.acpConfirmation.u.queryagentstatus.parameter,body);
						BuildAgentInfo(msg->event.acpConfirmation.u.queryagentstatus.destAgentInfo,body);
						break;
					case ACP_QueryAgentCallInfo_CONF:
						BuildGeneralConf(msg->event.acpConfirmation.u.queryAgentCallInfo.parameter,body);
						BuildAgentInfo(msg->event.acpConfirmation.u.queryAgentCallInfo.destAgentInfo,body);
						BuildCallinfo(msg->event.acpConfirmation.u.queryAgentCallInfo.callinfo,body);
						break;
					//alerting
					case ACP_OnOrigated:
						BuildGeneralEventReport(msg->event.acpEventReport.u.origatedEventReport.parameter,body);
						BuildCallinfo(msg->event.acpEventReport.u.origatedEventReport.callInfo,body);	
						break;
					case ACP_OnAnswerRequest:
						BuildAnswerRequestEventReport(*msg,body);
						break;
					//CallingNo="" CalledNo=""
					case ACP_OnHangupCallInConf:
						BuildHangupCallInfo(msg->event.acpEventReport.u.hangupCallEventReport,body);
						break;
					case ACP_OnBeginRecordSuccess:
						BuildRecordInfo(msg->event.acpEventReport.u.recordInfoEventReport,body);
						break;
					case ACP_OnForceIdle:
					case ACP_OnForceBusy:
					case ACP_OnForceOut:
						break;
					case ACP_GetCallData_CONF:
						BuildGeneralEventReport(msg->event.acpConfirmation.u.getCallData.parameter,body);
						/*
						TXMLHandle hCallData   = xmlGetElememt(body, "callData");
						if(hCallData)
						{
							const char* callData = sXmlGetValue(hCallData);
							if(callData)
								strcpy(msg.event.acpConfirmation.u.getCallData.callData,callData);
						}*/
						break;
					case ACP_OnRequestRelease:
					case ACP_OnHoldCallRelease:
						BuildReleaseEventReport(*msg,body);
						break;
					//include callinfo
					case ACP_OnReturnFromIVR:
					case ACP_OnConsultationBack:
					case ACP_OnReturnFromPhone:
					case ACP_OnReleaseSuccess:
						BuildReleaseEventReport(*msg,body);
						BuildCallinfo(msg->event.acpEventReport.u.releaseEventReport.callInfo,body);
						break;
					case ACP_OnAnswerSuccess:
					case ACP_OnConsultationSuccess:
					case ACP_OnCallInsideSucTalk:
					case ACP_OnCallOutsideSuccTalk:
					case ACP_OnHoldSuccess:
					case ACP_OnRetrieveHoldSuccess:
					case ACP_OnConferenceSuccess:
					case ACP_OnConferenceSucTalk:
					case ACP_OnAlternateSuccess:
					case ACP_OnAlternated:
					case ACP_OnListenSuccess:
					case ACP_OnInsertSuccess:
					case ACP_OnInterceptSuccess:
					case ACP_OnIntercepted:
					case ACP_OnInserted:
					case ACP_OnSignOuted:
					case ACP_OnBridgeSuccess:
						BuildCallinfoEventReport(*msg,body);
						break;
					//no callinfo 
					case ACP_OnAnswerFailure:
					case ACP_OnUserHangupBeforeResponse:
					case ACP_OnReleaseFailure:
					case ACP_OnLongNoAnswer:
					case ACP_OnConsultationFailure:
					case ACP_OnTransSuccess:
					case ACP_OnTransFailure:	
	 				case ACP_OnSingleTransferSuccess:
					case ACP_OnSingleTransferFailure:
					case ACP_OnCallInsideSuccess:
					case ACP_OnCallInsideFailure:
					case ACP_OnCallOutsideSuccess:
					case ACP_OnCallOutsideFailure:
					case ACP_OnHoldFailure:
					case ACP_OnRetrieveHoldFailure:
					case ACP_OnConferenceFailure:
					case ACP_OnAlternateFailure:
					case ACP_OnListenFailure:
					case ACP_OnInsertFailure:
					case ACP_OnStopInsertSuccess:
					case ACP_OnStopInsertFailure:
					case ACP_OnInterceptFailure:
					case ACP_OnForceIdleSuccess:
					case ACP_OnForceIdleFailure:
					case ACP_OnForceBusySuccess:	
					case ACP_OnForceBusyFailure:	
					case ACP_OnForceOutSuccess:	
					case ACP_OnForceOutFailure:	
					case ACP_OnBeginRecordFailure:
					case ACP_OnStopRecordSuccess:
					case ACP_OnStopRecordFailure:
					case ACP_OnForceRelease:
					case ACP_OnAgentAllBusy:
					case ACP_OnStopListenSuccess:
					case ACP_OnStopListenFailure:
					case ACP_OnListened:
					case ACP_OnBridgeFailure:
					case ACP_OnSystemMessage:
						BuildGeneralEventReport(msg->event.acpEventReport.u.generalEventReport,body);
						break;
					case ACP_HEART_BEAT:
						break;					
					default:
	                    //ASSERT(FALSE);
						log()->ERROR("不认识的eventType");
						RELEASE_AND_EXIT
						break;
						
				}//end switch

              
			}
		}
	}
	RELEASE_AND_EXIT_0
}

/*
int CAgent::handle_message(const string& msg)
{
	log()->LOG("收到消息 %s",msg.c_str());
	size_t pos = msg.find("<acpMessage");
	if(pos == string::npos)
	{
		log()->ERROR("座席收到的消息格式错误(没有<acp),具体消息为 %s",msg.c_str());
		return -1;
	}
	msg = msg.substr(pos);

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
			//setStatus(Initial);
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
			return //setStatus(SignIn);
		}
		else if(strcmp("SignOut", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("signOut失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(SignOut);
		}
		else if(strcmp("SetBusy", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("setBusy失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(Busy);
			
		}
		else if(strcmp("SetIdle", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("setIdle失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(Idle);
		}
		else if(strcmp("AgentReconnect", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("AgentReconnect失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(SignIn);
		}

		else if(strcmp("ForceIdle", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("ForceIdle失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(Idle);
		}

		else if(strcmp("ForceBusy", name->value()) == 0)
		{
			
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("ForceBusy 失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(Busy);
		}

		else if(strcmp("ForceOut", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("forceOut失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(SignOut);
			
		}

		else if(strcmp("ReleaseCall", name->value()) == 0)
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("releaseCall失败，失败原因%s",desc->value());
				return -1;
			}
			return //setStatus(AfterCall);
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
			return //setStatus(Idle);
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
			return //setStatus(SignOut);
		}
		//所有座席忙
		else if(strcmp("OnAgentAllBusy", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnAgentAllBusy失败，失败原因%s",desc->value()));
				return -1;
			}
			return //setStatus(Busy);
			
		}

		//所有座席置闲
		if(strcmp("OnForceIdle",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceIdle失败，失败原因%s",desc->value()));
				return -1;
			}
			return //setStatus(Idle);
		}

		//所有座席置忙
		if(strcmp("OnForceBusy",name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceBusy失败，失败原因%s",desc->value()));
				return -1;
			}
			return //setStatus(Busy);
		}

		//所有座席签出
		else if(strcmp("OnForceOut", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceOut失败，失败原因%s",desc->value()));
				return -1;
			}
			return //setStatus(SignOut);
		}

		else if(strcmp("OnForceRelease", name->value()))
		{
			if(strcmp(code->value(),"0") != 0)
			{
				log()->ERROR("OnForceRelease失败，失败原因%s",desc->value()));
				return -1;
			}
			return //setStatus(AfterCall);
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
			return	//setStatus(Calling);
			
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
			return //setStatus(AfterCall);
		}
		return 0;
	}
}
*/

map<string,EventType_t> CAgent::eventTypeMap;
CAgent::CAgent()
{
	
	inteval = statusAfterCall = -1;
	m_initial_sock = -1;
	m_signIn_sock = -1;
	m_connected = -1;
	m_sockState = 0;
	m_total_call_num = 0;
	m_success_call_num = 0;
	m_ready = 0;
	m_is_sign_in = false;
	

	memset(m_deviceID, 0, sizeof(m_deviceID));
	memset(m_agentID, 0, sizeof(m_agentID));
	memset(m_sessionID, 0, sizeof(m_sessionID));
	memset(m_vccID, 0, sizeof(m_vccID));
	memset(m_passwd, 0, sizeof(m_passwd);
	memset(m_acpEvent, 0, sizeof(m_acpEvent));
	memset(m_timeStamp, 0, sizeof(m_timeStamp));

	while(m_initial_msgToSend.empty() == false)
	{
		m_initial_msgToSend.pop();
	}
	while(m_signIn_msgToSend.empty() == false)
	{
		m_signIn_msgToSend.pop();
	}
	if(CAgent::eventTypeMap.empty()==true)
	{
		CAgent::eventTypeMap.insert(make_pair("HeartBeat", ACP_HEART_BEAT));	
		CAgent::eventTypeMap.insert(make_pair("Initial", ACP_Initial_CONF));	
		CAgent::eventTypeMap.insert(make_pair("SignIn", ACP_SignIn_CONF));	
		CAgent::eventTypeMap.insert(make_pair("SignOut", ACP_SignOut_CONF));
		CAgent::eventTypeMap.insert(make_pair("SetBusy", ACP_SetBusy_CONF));	
		CAgent::eventTypeMap.insert(make_pair("SetIdle", ACP_SetIdle_CONF));	
		CAgent::eventTypeMap.insert(make_pair("AgentReport", ACP_AgentReport_CONF));	
		CAgent::eventTypeMap.insert(make_pair("SetAgentAutoEnterIdle", ACP_SetAgentAutoEnterIdle_CONF));
		CAgent::eventTypeMap.insert(make_pair("AgentEnterIdle", ACP_AgentEnterIdle_CONF));	
		CAgent::eventTypeMap.insert(make_pair("AgentEnterWrapUp", ACP_AgentEnterWrapUp_CONF));	
		CAgent::eventTypeMap.insert(make_pair("ForceIdle", ACP_ForceIdle_CONF));
		CAgent::eventTypeMap.insert(make_pair("ForceBusy", ACP_ForceBusy_CONF));	
		CAgent::eventTypeMap.insert(make_pair("ForceOut", ACP_ForceOut_CONF));	
		CAgent::eventTypeMap.insert(make_pair("ReleaseCall", ACP_ReleaseCall_CONF));	
		CAgent::eventTypeMap.insert(make_pair("ReleaseCallByCallID", ACP_ReleaseCallByCallID_CONF));	
		CAgent::eventTypeMap.insert(make_pair("SendDTMF", ACP_SendDTMF_CONF));
		CAgent::eventTypeMap.insert(make_pair("CallInside", ACP_CallInside_CONF));
		CAgent::eventTypeMap.insert(make_pair("Consultation", ACP_Consultation_CONF));
		CAgent::eventTypeMap.insert(make_pair("CallOutside", ACP_CallOutside_CONF));
		CAgent::eventTypeMap.insert(make_pair("SingleStepTransfer", ACP_SingleStepTransfer_CONF));

		CAgent::eventTypeMap.insert(make_pair("Transfer", ACP_Transfer_CONF));
		CAgent::eventTypeMap.insert(make_pair("SetTransfer", ACP_SetTransfer_CONF));
		CAgent::eventTypeMap.insert(make_pair("Hold", ACP_Hold_CONF));
		CAgent::eventTypeMap.insert(make_pair("RetrieveHold", ACP_RetrieveHold_CONF));
		CAgent::eventTypeMap.insert(make_pair("Conference", ACP_Conference_CONF));
		CAgent::eventTypeMap.insert(make_pair("Alternate", ACP_Alternate_CONF));
		CAgent::eventTypeMap.insert(make_pair("DoReplyMultimeAndHangUpCall", ACP_DoReplyMultimeAndHangUpCall_CONF));
		CAgent::eventTypeMap.insert(make_pair("Listen", ACP_Listen_CONF));
		CAgent::eventTypeMap.insert(make_pair("Intercept", ACP_Intercept_CONF));
		CAgent::eventTypeMap.insert(make_pair("Insert", ACP_Insert_CONF));
		CAgent::eventTypeMap.insert(make_pair("StopInsert", ACP_StopInsert_CONF));
		CAgent::eventTypeMap.insert(make_pair("BeginRecord", ACP_BeginRecord_CONF));
		CAgent::eventTypeMap.insert(make_pair("StopRecord", ACP_StopRecord_CONF));
		CAgent::eventTypeMap.insert(make_pair("Bridge", ACP_Bridge_CONF));
		CAgent::eventTypeMap.insert(make_pair("SetCallData", ACP_SetCallData_CONF));
		CAgent::eventTypeMap.insert(make_pair("GetCallData", ACP_GetCallData_CONF));
		CAgent::eventTypeMap.insert(make_pair("QueryTotalQueueWaitNum", ACP_QueryTotalQueueWaitNum_CONF));
		CAgent::eventTypeMap.insert(make_pair("QueryQueueWaitNum", ACP_QueryQueueWaitNum_CONF));
		CAgent::eventTypeMap.insert(make_pair("QueryAgentStatus", ACP_QueryAgentStatus_CONF));
		CAgent::eventTypeMap.insert(make_pair("GetAccessNo", ACP_GetAccessNo_CONF));
		CAgent::eventTypeMap.insert(make_pair("QueryAgentCallInfo", ACP_QueryAgentCallInfo_CONF));
		CAgent::eventTypeMap.insert(make_pair("SetCTIParam", ACP_SetCTIParam_CONF));

		
		CAgent::eventTypeMap.insert(make_pair("OnOrigated", ACP_OnOrigated));
		CAgent::eventTypeMap.insert(make_pair("OnAnswerRequest", ACP_OnAnswerRequest));
		CAgent::eventTypeMap.insert(make_pair("OnAnswerSuccess", ACP_OnAnswerSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnAnswerFailure", ACP_OnAnswerFailure));
		CAgent::eventTypeMap.insert(make_pair("OnUserHangupBeforeResponse", ACP_OnUserHangupBeforeResponse));
		CAgent::eventTypeMap.insert(make_pair("OnRequestRelease", ACP_OnRequestRelease));	
		CAgent::eventTypeMap.insert(make_pair("OnReleaseSuccess", ACP_OnReleaseSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnReleaseFailure", ACP_OnReleaseFailure));
		CAgent::eventTypeMap.insert(make_pair("OnLongNoAnswer", ACP_OnLongNoAnswer));
		CAgent::eventTypeMap.insert(make_pair("OnConsultationBack", ACP_OnConsultationBack));
		CAgent::eventTypeMap.insert(make_pair("OnConsultationSuccess", ACP_OnConsultationSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnConsultationFailure", ACP_OnConsultationFailure));
		CAgent::eventTypeMap.insert(make_pair("OnTransSuccess", ACP_OnTransSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnTransFailure", ACP_OnTransFailure));
		CAgent::eventTypeMap.insert(make_pair("OnReturnFromIVR", ACP_OnReturnFromIVR));
		CAgent::eventTypeMap.insert(make_pair("OnReturnFromPhone", ACP_OnReturnFromPhone));
		CAgent::eventTypeMap.insert(make_pair("OnBridgeSuccess", ACP_OnBridgeSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnBridgeFailure", ACP_OnBridgeFailure));

		
		CAgent::eventTypeMap.insert(make_pair("OnSingleTransferSuccess", ACP_OnSingleTransferSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnSingleTransferFailure", ACP_OnSingleTransferFailure));
		CAgent::eventTypeMap.insert(make_pair("OnCallInsideSuccess", ACP_OnCallInsideSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnCallInsideFailure", ACP_OnCallInsideFailure));
		CAgent::eventTypeMap.insert(make_pair("OnCallInsideSucTalk", ACP_OnCallInsideSucTalk));
		CAgent::eventTypeMap.insert(make_pair("OnCallOutsideSuccess", ACP_OnCallOutsideSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnCallOutsideFailure", ACP_OnCallOutsideFailure));
		CAgent::eventTypeMap.insert(make_pair("OnCallOutsideSuccTalk", ACP_OnCallOutsideSuccTalk));
		CAgent::eventTypeMap.insert(make_pair("OnHoldSuccess", ACP_OnHoldSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnHoldFailure", ACP_OnHoldFailure));
		CAgent::eventTypeMap.insert(make_pair("OnRetrieveHoldSuccess" ACP_OnRetrieveHoldSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnRetrieveHoldFailure", ACP_OnRetrieveHoldFailure));
		CAgent::eventTypeMap.insert(make_pair("OnHoldCallRelease", ACP_OnHoldCallRelease));
		CAgent::eventTypeMap.insert(make_pair("OnConferenceSuccess", ACP_OnConferenceSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnConferenceFailure", ACP_OnConferenceFailure));
		CAgent::eventTypeMap.insert(make_pair("OnConferenceSucTalk", ACP_OnConferenceSucTalk));
		CAgent::eventTypeMap.insert(make_pair("OnHangupCallInConf", ACP_OnHangupCallInConf));
		CAgent::eventTypeMap.insert(make_pair("OnAlternateSuccess", ACP_OnAlternateSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnAlternated", ACP_OnAlternated));
		CAgent::eventTypeMap.insert(make_pair("OnAlternateFailure", ACP_OnAlternateFailure));
		
		CAgent::eventTypeMap.insert(make_pair("OnListenSuccess", ACP_OnListenSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnListenFailure", ACP_OnListenFailure));
		CAgent::eventTypeMap.insert(make_pair("OnInsertSuccess", ACP_OnInsertSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnInsertFailure", ACP_OnInsertFailure));
		CAgent::eventTypeMap.insert(make_pair("OnStopInsertSuccess", ACP_OnStopInsertSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnStopInsertFailure", ACP_OnStopInsertFailure));
		CAgent::eventTypeMap.insert(make_pair("OnInterceptSuccess", ACP_OnInterceptSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnInterceptFailure", ACP_OnInterceptFailure));
		CAgent::eventTypeMap.insert(make_pair("OnForceIdleSuccess", ACP_OnForceIdleSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnForceIdleFailure", ACP_OnForceIdleFailure));
		
		
		CAgent::eventTypeMap.insert(make_pair("OnForceBusySuccess", ACP_OnForceBusySuccess));
		CAgent::eventTypeMap.insert(make_pair("OnForceBusyFailure", ACP_OnForceBusyFailure));
		CAgent::eventTypeMap.insert(make_pair("OnForceOutSuccess", ACP_OnForceOutSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnForceOutFailure", ACP_OnForceOutFailure));
		CAgent::eventTypeMap.insert(make_pair("OnBeginRecordSuccess", ACP_OnBeginRecordSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnBeginRecordFailure", ACP_OnBeginRecordFailure));
		CAgent::eventTypeMap.insert(make_pair("OnStopRecordSuccess", ACP_OnStopRecordSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnStopRecordFailure", ACP_OnStopRecordFailure));
		CAgent::eventTypeMap.insert(make_pair("OnForceIdle", ACP_OnForceIdle));
		CAgent::eventTypeMap.insert(make_pair("OnForceBusy", ACP_OnForceBusy));
		CAgent::eventTypeMap.insert(make_pair("OnForceOut", ACP_OnForceOut));
		CAgent::eventTypeMap.insert(make_pair("OnSignOuted", ACP_OnSignOuted));
		CAgent::eventTypeMap.insert(make_pair("OnForceRelease", ACP_OnForceRelease));
		CAgent::eventTypeMap.insert(make_pair("OnAgentAllBusy", ACP_OnAgentAllBusy));
		CAgent::eventTypeMap.insert(make_pair("OnIntercepted", ACP_OnIntercepted));
		CAgent::eventTypeMap.insert(make_pair("OnInserted", ACP_OnInserted));
		CAgent::eventTypeMap.insert(make_pair("OnStopListenSuccess", ACP_OnStopListenSuccess));
		CAgent::eventTypeMap.insert(make_pair("OnStopListenFailure", ACP_OnStopListenFailure));
		CAgent::eventTypeMap.insert(make_pair("OnListened", ACP_OnListened));
		CAgent::eventTypeMap.insert(make_pair("OnSystemIdle", ACP_OnListened));
		CAgent::eventTypeMap.insert(make_pair("OnQueueReport", ACP_OnQueueReport));
		CAgent::eventTypeMap.insert(make_pair("OnSystemMessage", ACP_OnSystemMessage));

		
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
	
	
	log()->LOG("座席发送Initial消息：%s",msg);
	//setStatus(Try2Initial);
	return sendMsgEx(msg,"Initial");
}

int CAgent::signIn()
{
	sockState = 0;
	
	
	char msg[300];
	sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SignIn\">"
 		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter deviceID=\"%s\" passwd=\"%s\" ready=\"%d\"/>"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID,m_deviceID,m_passwd,m_ready);
	log()->LOG("座席发送signin消息：%s",msg);
    
    //setStatus(Try2SignIn);
	return sendMsgEx(msg,"SignIn");

}


int CAgent::signOut()
{

	char msg[300];
	sprintf(msg, "<acpMessage ver=\"2.0.0\">"
        "<header><sessionID>%s</sessionID></header>"
        "<body type=\"request\" name=\"SignOut\">"
		"<agent vccID=\"%s\" agentID=\"%s\" deviceID=\"%s\"/>"
        "<parameter />"
        "</body></acpMessage>", m_sessionID ,m_vccID,m_agentID,m_deviceID);
	
	log()->LOG("座席发送signOut消息：%s",msg);
	//setStatus(Try2SignOut);
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
   	
	log()->LOG("座席发送setIdle消息：%s",msg);
	//setStatus(Try2SetIdle);
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
   	
	log()->LOG("座席发送setBusy消息：%s",msg);
	//setStatus(Try2SetBusy);
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
    
      
   	
	log()->LOG("座席发送forceBusy消息：%s",msg);
	//setStatus(Try2ForceBusy);
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
    
      
   	
	log()->LOG("座席发送forceIdle消息：%s",msg);
	//setStatus(Try2ForceIdle);
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
    
	log()->LOG("座席发送forceOut消息：%s",msg);
	//setStatus(Try2ForceOut);
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
	log()->LOG("座席发送releaseCall消息：%s",msg);
	//setStatus(Try2ReleaseCall);
	return sendMsgEx(msg,"ReleaseCall");
}





