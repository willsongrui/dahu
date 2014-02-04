#include <simulation.h>
#include <utils.h>
#include <string>
#include <xmlapi.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>

#define MAX_EVENTS 1000
int epollfd; 						 //EPOLL句柄
CCenter center; 					 //呼叫中心类,包括连接web服务器的socket
map<int,string> socket_agentID_map;  //socket和agent的映射map
map<string,CAgent>agentID_agent_map;
queue<int> socket_Not_In_Epoll;		 //还未加入到EPOLL中的socket
CConf conf;
LOG* simu_log;						
int listenWeb;						 //ipc的socket句柄


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
	
	if(create_agents() < 0)
	{
		simu_log->ERROR("初始化座席失败");
		return -1;
	}
	epollfd = epoll_create(MAX_EVENTS);
	if(epollfd < 0)
	{
		simu_log->ERROR("EPOLL 错误");
		return 0;
	}
	
	//监听web服务器
	listenWeb = create_connection_to_web(conf.webSocket);
	if(listenWeb < 0)
	{
		simu_log->ERROR("IPC socket错误");
		return 0;
	}
	socket_Not_In_Epoll.push(listenWeb);

	

	while(true)
	{
		
		struct epoll_event ev,events[MAX_EVENTS];
		while(socket_Not_In_Epoll.size()!=0)
		{
			memset(ev,0,sizeof(ev));
			int sock = socket_Not_In_Epoll.front();
			socket_Not_In_Epoll.pop();
			ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
			ev.data.fd = sock;
			if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock,&ev)<0)
			{
				char errorMsg[100];
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
			if(ev.data.fd == listenWeb)			//listenWeb是监听的进程间通信的句柄
			{
				struct sockaddr_un un;
				int len = sizeof(un); 
				int ret = accept(listenWeb, (struct sockaddr *)&un, &len));
   				if (ret < 0) 
   				{

   					simu_log->ERROR("IPC Sock Connection 错误, 错误原因 %s ",strerror(errno));
      				return 0;     
   				}
   				if(setnonblocking(ret) < 0)
   				{
   					simu_log->ERROR("设置成nonblocking模式错误");
   					return 0;
   				}
   				ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
   				ev.data.fd = ret;
   				
   				if(epoll_ctl(epollfd,EPOLL_CTL_ADD,ret,&ev) == -1)
   				{
   				
   					simu_log->ERROR("套接字(listenWeb) %d 加入EPOLL错误, 错误原因 %s ",ret,strerror(errno));
   					return 0;
   				}
   				queue <string> tempQ;
   				center.webSocket.append(make_pair(ret,tempQ));
			}
			
			else
			{
				if(ev.events & EPOLLERR)
				{
					simu_log->ERROR("套接字 %d 错误, 错误原因 %s ",ev.data.fd,strerror(errno));
					close_sock_in_epoll(ev.data.fd)
					
				}
				else if(ev.events & EPOLLIN)
				{
					char buf[2000];
					int ret = recv(ev.data.fd,buf,sizeof(buf),0);
					if(ret==0)
					{
						simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
						close_sock_in_epoll(ev.data.fd);
					}
					else if(ret < 0)
					{
						simu_log->ERROR("套接字 %d 接收错误，错误原因 %s",ev.data.fd,strerror(errno));
					}
					else
					{
						handle_massage(ev.data.fd,string(buf));		//根据socket内容处理message内容
					}
				}
				else if(ev.events & EPOLLOUT)
				{
					send_message(ev.data.fd);
				}
				else if(ev.events & EPOLLRDHUP)
				{
					simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
					close_sock_in_epoll(ev.data.fd);
				}
				else if(ev.events & EPOLLHUP)
				{
					simu_log->LOG("套接字 %d 对方关闭连接",ev.data.fd);
					close_sock_in_epoll(ev.data.fd);
				}
			}
		}
	}




}
