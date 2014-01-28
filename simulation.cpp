#include <simulation.h>
#include <simu_structure.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;
extern CCenter center;
extern map<int,string> socket_agentID_map;
extern map<string,CAgent> agentID_agent_map;
extern int epollfd;
extern LOG* simu_log;
extern CConf conf;


int create_agents()
{
	for(int i=0; i < conf.agentNum; i++)
	{
		CAgent* agent = new CAgent();
		agent->initial_ip = conf.ctiIP;
		agent->port = ctiPort;
		agent->agentID = 

	}
}

int load_config(string confFile)
{
	fs = open(confFile.c_str(),O_RDONLY);
	if(fs < 0)
	{
		simu_log->ERROR("打开配置文件 %s 出错,错误原因: %s",confFile.c_str(),strerror(errno));
		return -1;
	}
	char config[1000];
	if(read(fs,config,sizeof(config)) < 0)
	{
		simu_log->ERROR("读取配置文件错误，错误原因%s",strerror(errno));
		return -1;
	}
	xml_document<> doc;
	try
	{
		doc.parse<0>(config);
	}
	catch (exception e)
	{
		simu_log->ERROR("配置文件格式错误");
		return -1;
	}
	xml_node<>* root;
	
	xml_node<>* webSocket;
	xml_node<>* ctiIP;
	xml_node<>* ctiPort;
	xml_node<>* vccID;
	xml_node<>* agentNum;
	xml_node<>* agentID;

	root = doc.first_node();
	if(!root)
	{
		simu_log->ERROR("配置文件没有根节点");
		return -1;
	}

	
	webSocket = root->first_node("webSocket");
	ctiIP = root->first_node("ctiIP");
	ctiPort = root->first_node("ctiPort");
	vccID = root->first_node("vccID");
	agentNum = root->first_node("agentNum");
	agentID = root->first_node("agentID");
		
	if(!(root && webSocket && ctiIP && ctiPort && vccID && agentNum && agentID))
	{
		simu_log->ERROR("配置文件节点不全");
		return -1;	
	}
	
	conf.webSocket = string(webSocket->value());
	conf.ctiIP = string(ctiIP->value());
	conf.ctiPort = atoi(ctiPort->value());
	conf.vccID = string(vccID->value());
	conf.agentNum = atoi(agentNum->value());
	conf.agentID = string(agentID->value());
	simu_log->LOG("配置文件读取成功");
	return 0;
}

int all_initial()
{

	map <string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		if((iter->second).initial() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席initial失败",iter->first);
			return -1;
		}
	}
	return 0;	
}

int all_signIn()
{
	map <string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		if((iter->second).signIn() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席signIn失败",iter->first);
			return -1;
		}
	}
	return 0;

}

int all_setIdle()
{
	map <string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		if((iter->second).setIdle() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席setIdle失败",iter->first);
			return -1;
		}
	}
	return 0;

}

int all_setBusy()
{
	map <string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		if((iter->second).setBusy() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席setBusy失败",iter->first);
			return -1;
		}
	}
	return 0;

}

int all_signOut()
{
	map <string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		if((iter->second).signOut() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席signOut失败",iter->first);
			return -1;
		}
	}
	return 0;

}

int all_report(int sockFd)
{
	string report_message = string(conf.agentNum);
	map<string, CAgent>::iterator iter;
	for(iter = agentID_agent_map.begin(); iter != agentID_agent_map.end(); iter++)
	{
		message = message + " " + string(iter->sockState) + " " + string(iter->curState) + " " + string(iter->successCall) + " " + string(iter->totalCall);
	}
	iter = center.webSocket.find(sockFd);
	if(iter == center.webSocket.end())
	{
		simu_log->ERROR("服务器套接字 %d 没有找到",sockFd);
		return -1;
	}
	(iter->second).push(message);
	simu_log->LOG("向服务器套接字 %d 发送report，内容为%s",sockFd,message.c_str());
	return 0;
}





int handle_web_message(int sockFd,string message)
{
	simu_log->LOG("收到服务器套接字 %d 的消息，内容为%s",sockFd,message.c_str());
	if(message == "ALL_INITIAL")
	{
		return all_initial();
	}
	else if(message == "ALL_SIGNIN")
	{
		return all_signIn();
	}
	else if(message == "ALL_SETIDLE")
	{
		return all_setIdle();
	}
	else if(message == "ALL_SETBUSY")
	{
		return all_setBusy();
	}
	else if(message == "ALL_SIGNOUT")
	{
		return all_signOut();
	}
	else if(message == "ALL_REPORT")
	{
		return all_report(sockFd);
	}
	else{
		simu_log -> ERROR("不认识的命令: %s",message.c_str());
		return -1;
	}
	return 0;
}


int handle_message(int sockFd,string message)
{
	if(center.webSocket.find(sockFd)!=center.webSocket.end())		//webSocket是接收web命令的套接字
	{
		return handle_web_message(sockFd,message);
	}
	else
	{
		CAgent agent = find_agent(sockFd);
		if(agent->exist == false)
		{
			simu_log->LOG("no agent has fd %d when closing",sockFd);
			return -1;
		}
		else
		{
			return agent->handle_message(message);
		}
		
	}
	
}


int send_message(int sockFd);
{
	map<int,queue<string> >::iterator it;
	it = center.webSocket.find(sockFd);
	if(it != center.webSocket.end())			//此套接字是webSocket
	{
		while((it->second).empty()!=false)
		{
			if(send(sockFd,(it->second).front) < 0)
			{
				simu_log->ERROR("套接字 %d 向web发送消息 %s 错误,错误原因是 : %s",sockFd,(it->second).front,strerror(errno));
			}
			(it->second).pop();
		}
	}
	else
	{
		CAgent* agent = find_agent(sockFd);
		if(agent.exist == false)
		{
			simu_log->ERROR("套接字 %d 既不是座席也不是web",sockFd);
			return -1;
		}
		else
		{
			if(agent->send_message() < 0)
			{
				simu_log->ERROR("座席 %s (套接字 %d)发送消息错误",agent->agentID,sockFd);
				return -1;
			}
		}
	}
	return 0;
}

int non_block_connect(string ip,int port)
{
	sockFd = socket(AF_INET,DATA_STREAM,0);
	if(sockFd < 0)
	{
		simu_log->ERROR("error create socket %s\n",strerror(errno));
		return -1;
	}
	int flags = fcntl(sockFd,F_GETFL);
	if(flags < 0)
	{
		simu_log->ERROR("error get flag for %d : %s\n",sockFd,strerror(errno));
		return -1;	
	}
	if(fcntl(sockFd,F_SETFL,flags|O_NONBLOCK) < 0)
	{
		simu_log->ERROR("error set flag to O_NONBLOCK for %d :%s",sockFd,strerror(errno));
		return -1;
	}
	return 0;
}


int close_sock_in_epoll(int sockFd)
{
	if(EPOLL_CTL(epollfd,EPOLL_CTL_DEL,sockFd,NULL) < 0)
	{
		
		simu_log->LOG("%s while deleting fd %d",strerror(errno),sockFd);
		return -1;
	}
	if(center.webSocket.erase(center.webSocket.find(sockFd)) != 0)
	{
		//sockFd是监听web套接字
		return 0;
	}
	
	CAgent* agent = find_agent(sockFd);
	if(agent.exist == false)
	{
		simu_log->LOG("no agent has fd %d when closing",sockFd);
		return -1;
	}
	else
	{
		agent->LOG("Socket Closed");
		agent->sockState = 0;
		agent->preState = agent->curState;
		agent->curState = LogOut;
		
	}
	return 0;
}

int create_connection_to_web(string webSocket)
{
	sockFd = socket(AF_UNIX,SOCK_STREAM,0);
	if(sockFd < 0)
	{
		simu_log->ERROR("创建web套接字失败,失败原因: %s",strerror(errno));
		return -1;
	}
	int flags = fcntl(sockFd,F_GETFL);
	if(flags < 0)
	{
		simu_log->ERROR("得到web套接字flag失败,失败原因: %s",strerror(errno));
		return -1;	
	}
	if(fcntl(sockFd,F_SETFL,flags|O_NONBLOCK) < 0)
	{
		simu_log->ERROR("设置web套接字为非阻塞模式失败,失败原因: %s",strerror(errno));
		return -1;
	}
	if(listen(sockFd,1000) < 0)
	{
		simu_log->ERROR("监听web套接字失败,错误原因: %s",strerror(errno));
		return -1;
	}
	return sockFd;
	
}
























