#include <simulation.h>
#include <simu_structure.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
extern CCenter center;
extern map<int,string> socket_agentID_Map;
extern int epollfd;
extern LOG* simu_log;
extern CConf conf;


int load_config(string confFile)
{
	
	xmlDocPtr pDoc = NULL;
	xmlNodePtr pRoot = NULL;
	xmlNodePtr curNode = NULL;

	pDoc = xmlReadFile(confFile.c_str(), "UTF-8", XML_PARSE_REVOVER);
	if(pDoc == NULL)
	{
		simu_log->ERROR("打开配置文件 %s 出错",confFile.c_str());
		return -1;
	}
	pRoot = xmlDocGetRootElement(pDoc);
	if(pRoot == NULL)
	{
		simu_log->ERROR("%s 是空文档（没有root节点）", confFile.c_str());
		return -1;
	}
	if(xmlStrcmp(pRoot->name, BAD_CAST "config")!=0)
	{
		simu_log->ERROR("配置文件内容错误");
		return -1;
	}
	
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	context = xmlXPathNewContext(pDoc);
	
	if(context == NULL)
	{
		simu_log->ERROR("配置文件为空");
		return -1;
	}
	result = xmlXPathEvalExpression("/config/logFile",context);
	if(result == NULL)
	{
		simu_log->ERROR("logFile 错误");
		return -1;
	}
	else
	{
		conf.logFile = result->nodesetval;
	}
	
	result = xmlXPathEvalExpression("/config/webSocket",context);
	if(result == NULL)
	{
		simu_log->ERROR("webSocket 错误");
		return -1;
	}
	else
	{
		conf.webSocket = result->nodesetval;
	}

	result = xmlXPathEvalExpression("/config/ctiIP",context);
	if(result == NULL)
	{
		simu_log->ERROR("ctiIP 错误");
		return -1;
	}
	else
	{
		conf.ctiIP = result->nodesetval;
	}

	result = xmlXPathEvalExpression("/config/ctiPort",context);
	if(result == NULL)
	{
		simu_log->ERROR("ctiPort 错误");
		return -1;
	}
	else
	{
		conf.ctiPort = result->nodesetval;
	}

	result = xmlXPathEvalExpression("/config/vccID",context);
	if(result == NULL)
	{
		simu_log->ERROR("vccID 错误");
		return -1;
	}
	else
	{
		conf.vccID = result->nodesetval;
	}

	result = xmlXPathEvalExpression("/config/agentNum",context);
	if(result == NULL)
	{
		simu_log->ERROR("agentNum 错误");
		return -1;
	}
	else
	{
		conf.agentNum = result->nodesetval;
	}

	result = xmlXPathEvalExpression("/config/agentID",context);
	if(result == NULL)
	{
		simu_log->ERROR("agentID 错误");
		return -1;
	}
	else
	{
		conf.agentID = result->nodesetval;
	}

	simu_log->LOG("配置文件读取成功");
	return 0;
}

int all_initial()
{
	
}

int handle_web_message(int sockFd,string message)
{
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
		return all_report();
	}
	else{
		simu_log -> ERROR("不认识的命令: %s",message.c_str());
		return -1;
	}
}


int handle_message(int sockFd,string message)
{
	if(center.webSocket.find(sockFd)!=center.webSocket.end)		//webSocket是接收web命令的套接字
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
	if(it != center.webSocket.end)			//此套接字是webSocket
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
























