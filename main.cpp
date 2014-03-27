#include "simulation.h"
#include "simu_def.h"
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <queue>

#define MAX_EVENTS 1000


int epollfd; 						 //EPOLL句柄
CCenter center; 					 //呼叫中心类,包括连接web服务器的socket
//map<int,string> socket_agentID_map;  //socket和agent的映射map
//map<string,CAgent*> agentID_agent_map;
//queue<int> socket_Not_In_Epoll;		 //还未加入到EPOLL中的socket
CConf conf;
CLOG* simu_log;						

using namespace std;
void agentReportAlarm()
{

	map<string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if(iter->second->m_is_sign_in == true)
		{
			iter->second->agentReport();
		}
	}
	alarm(110);

}
int main()
{
	//系统的日志文件
	try
	{
		simu_log = new CLOG("simulation.log");
	}
	catch(int e)
	{
		printf("打开文件日志失败\n");
		return -1;
	}
	if(load_config("simulation.conf")<0)
	{
		printf("打开配置文件错误\n");
		return -1;
	}

	epollfd = epoll_create(MAX_EVENTS);
	if(epollfd < 0)
	{
		simu_log->ERROR("EPOLL 错误");
		return -1;
	}
	simu_log->LOG("EPOLL加载成功");
	
	//监听web服务器
	int listenWeb = create_connection_to_web(conf.webPort);
	if(listenWeb < 0)
	{
		simu_log->ERROR("IPC socket错误");
		return 0;
	}
	center.socket_Not_In_Epoll.push(listenWeb);



	if(create_agents() < 0)
	{
		simu_log->ERROR("初始化座席失败");
		return -1;
	}
	
	struct epoll_event ev,events[MAX_EVENTS];
	while(true)
	{
		while(center.socket_Not_In_Epoll.empty()!=0)
		{
			memset(&ev,0,sizeof(ev));
			int sock = center.socket_Not_In_Epoll.front();
			center.socket_Not_In_Epoll.pop();
			ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
			ev.data.fd = sock;
			if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock,&ev)<0)
			{
				simu_log->ERROR("套接字 %d 加入EPOLL错误, 错误原因 %s ",sock, strerror(errno));
			}
		}

		int nfds = epoll_wait(epollfd,events,MAX_EVENTS,10);
		if(nfds == -1)
		{
			simu_log->ERROR("epoll_wait 错误, 错误原因 %s ",strerror(errno));
			return 0;
		}
		for(int n = 0; n < nfds; ++n)
		{
			ev = events[n];
			if(ev.events & EPOLLIN)
			{
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

					char buf[2000];
					int ret = recv(ev.data.fd,buf,sizeof(buf),0);
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
						while((ret = recv(ev.data.fd,buf,sizeof(buf),0)) > 0)
						{
							msg = msg + string(buf);
						}
						handle_message(ev.data.fd, msg);
					}
				}
			}
			else if(ev.events & EPOLLOUT)
			{
				send_message(ev.data.fd);
			}
			else if(ev.events & EPOLLRDHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
				close_sock_and_erase(ev.data.fd);
			}
			else if(ev.events & EPOLLHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
				close_sock_and_erase(ev.data.fd);
			}
			else if(ev.events & EPOLLERR)
			{
				simu_log->ERROR("套接字 %d 错误, 错误原因 %s ",ev.data.fd,strerror(errno));
				close_sock_and_erase(ev.data.fd);
					
			}
			
		}
	}

}
