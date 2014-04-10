#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <string>
#include <queue>
#include "simulation.h"
#include "simu_def.h"

#define MAX_EVENTS 1000


int epollfd; 						 //EPOLL句柄
CCenter center; 					 //呼叫中心类,包括连接web服务器的socket
CConf conf;
CLOG* simu_log;						

using namespace std;
void agentReportAlarm(int signo)
{

	if(NULL != simu_log)
	{
		simu_log->LOG("进入alarm中");
	}
	//simu_log->LOG("进入alarm中");
	map<string, CAgent*>::iterator iter;

	int total_agent_num = 0;
	int total_SIGNIN_num = 0;
	
	memset(center.agent_state_array, 0, sizeof(center.agent_state_array));
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if(iter->second->m_is_sign_in == true)
		{
			iter->second->agentReport();
			total_SIGNIN_num++;
		}

		total_agent_num++;
		string state = center.detail_state_dict[iter->second->m_curState];
		simu_log->INFO("当前座席ID %s 状态为 %s，initial_socket为 %d， sign_socket为 %d", (iter->first).c_str(), state.c_str(), iter->second->m_initial_sock, iter->second->m_signIn_sock);
		center.agent_state_array[iter->second->m_curState]++;
	}
	for(int i = 0; i < static_cast<int>(AGENT_END); i++)
	{
		simu_log->LOG("当前共有%d个座席状态为：%s", center.agent_state_array[i], center.detail_state_dict[static_cast<DetailState_t>(i)].c_str());
	}
	

	map<int, string>::iterator sock_agentID_iter;
	for(sock_agentID_iter = center.socket_agentID_map.begin(); sock_agentID_iter != center.socket_agentID_map.end(); sock_agentID_iter++)
	{	
		simu_log->INFO("当前socekt %d 对应于座席ID为 %s ", sock_agentID_iter->first, (sock_agentID_iter->second).c_str());
	}

	alarm(50);

}
int main()
{
	//系统的日志文件
	//unlink("simulation.log");
	try
	{
		simu_log = new CLOG("simulation.log");
	}
	catch(int e)
	{
		printf("打开文件日志失败\n");
		return -1;
	}
	if(load_config("simulation.conf") < 0)
	{
		printf("打开配置文件错误\n");
		return -1;
	}
	simu_log->INFO("读取配置文件成功");
	//simu_log->ERROR("读取配置文件成功");

	epollfd = epoll_create(MAX_EVENTS);
	
	if(epollfd < 0)
	{
		simu_log->ERROR("EPOLL 错误");
		return -1; 
	}
	simu_log->INFO("EPOLL加载成功");
	
	//监听web服务器
	int listenWeb = create_connection_to_web(conf.webPort);

	if(listenWeb < 0)
	{
		simu_log->ERROR("web socket错误");
		return 0;
	}

	center.socket_Not_In_Epoll.push(listenWeb);

	if(create_agents() < 0)
	{
		simu_log->ERROR("初始化座席失败");
		return -1;
	}
	
	struct epoll_event ev,events[MAX_EVENTS];
	
	if(signal(SIGALRM, agentReportAlarm) == SIG_ERR)
	{
		simu_log->ERROR("创建alarm失败");
	}
	alarm(5);
	simu_log->LOG("创建alarm成功");
	
	while(true)
	{
		//simu_log->LOG("进入大循环");
		while(!center.socket_Not_In_Epoll.empty())
		{
			memset(&ev, 0, sizeof(ev));
			int sock = center.socket_Not_In_Epoll.front();
			center.socket_Not_In_Epoll.pop();
			ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
			ev.data.fd = sock;
			if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) < 0)
			{
				simu_log->ERROR("套接字 %d 加入EPOLL错误, 错误原因 %s ",sock, strerror(errno));
			}
			else
			{
				simu_log->LOG("将socket为%d的连接加入到EPOLL中", sock);
			}
		}

		int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 10);
		if(nfds == -1)
		{
			if(errno == EINTR)
			{
				simu_log->ERROR("EINTR, continue");
				continue;
			}
			else
			{
				simu_log->ERROR("epoll_wait 错误, 错误原因 %s ", strerror(errno));
				return -1;
			}
		}
		for(int n = 0; n < nfds; ++n)
		{
			ev = events[n];
			if(ev.events & EPOLLERR)
			{
				simu_log->ERROR("套接字 %d 错误, 错误原因 %s ",ev.data.fd,strerror(errno));
				close_sock_and_erase(ev.data.fd);
					
			}

			else if(ev.events & EPOLLHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
				close_sock_and_erase(ev.data.fd);
			}

			else if(ev.events & EPOLLIN)
			{
				simu_log->LOG("socekt %d 可读", ev.data.fd);
				if(ev.data.fd == listenWeb)
				{
					struct sockaddr_in addr;
					socklen_t addrlen = sizeof(addr);
					int webfd = accept(listenWeb, (struct sockaddr*)(&addr), &addrlen);
					
					if(setnonblocking(webfd) < 0)
					{
						simu_log->ERROR("当试图将webSocket %d 设置为nonblock时失败", webfd);
						close(webfd);
						continue;
					}
					struct epoll_event event;
					event.data.fd = webfd;
					event.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
					if(epoll_ctl(epollfd, EPOLL_CTL_ADD, webfd, &event) < 0)
					{
						simu_log->ERROR("当试图将websocket %d加入epoll时失败，失败原因：%s", webfd, strerror(errno));
						close(webfd);
						continue;
					}
					queue<string> msg;
					center.webSocket.insert(make_pair(webfd, msg));
					simu_log->LOG("成功建立websocket %d连接", webfd);
				}
				else
				{
					simu_log->LOG("收到来自socket%d的消息", ev.data.fd);
					char buf[2000];
					int ret = recv(ev.data.fd, buf, sizeof(buf), 0);
					if(ret == 0) 
					{
						simu_log->LOG("socket %d 关闭", ev.data.fd);
						close_sock_and_erase(ev.data.fd);
					}
					else if(ret < 0)
					{
						simu_log->ERROR("套接字 %d 接收错误，错误原因 %s",ev.data.fd,strerror(errno));
					}
					else
					{
						string msg = string(buf);
						while((ret = recv(ev.data.fd, buf, sizeof(buf),0)) > 0)
						{
							msg = msg + string(buf);
						}
						handle_message(ev.data.fd, msg);
					}
				}
			}
			else if(ev.events & EPOLLOUT)
			{
				//simu_log->LOG("向socket %d发送消息");
				send_message(ev.data.fd);
			}
			else if(ev.events & EPOLLRDHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
				close_sock_and_erase(ev.data.fd);
			}
			
		}
	}
}
