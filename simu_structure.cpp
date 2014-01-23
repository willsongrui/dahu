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