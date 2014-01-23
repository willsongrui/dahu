#include <simulation.h>
#include <simu_structure.h>

using namespace std;
extern CCenter center;
extern map<int,string> socket_agentID_Map;
extern int epollfd;
extern LOG* simu_log;
extern CConf conf;


int handle_message(int sockFd,string message)
{
	if(center.webSocket.find(sockFd)!=center.webSocket.end)		//webSocket是接收web命令的套接字
	{
		if(handle_web_message(sockFd,message) < 0)
		{
			simu_log->ERROR("error in heandle_web_message, sockFd:%d ,message:%s",sockFd,message);
			return -1;
		}
		
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
	return 0;
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

int load_config(string conf)
{

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
		agent->log("Socket Closed");
		agent->sockState = 0;
		agent->preState = agent->curState;
		agent->curState = LogOut;
		
	}
	return 0;
}
