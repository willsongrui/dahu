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
const int alarm_sleep_interval = 20;
const int epoll_wait_num = 1000;
int epollfd; 						 //EPOLL句柄
CCenter center = CCenter(); 					 //呼叫中心类,包括连接web服务器的socket
CConf* conf= new CConf();
CLOG* simu_log;						
//int enter_alarm_time = 0;
using namespace std;
void agentReportAlarm(int signo)
{
	//enter_alarm_time += 1;
	/*if(enter_alarm_time > 1000000)
	{
		enter_alarm_time = 3;
	}*/
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
		simu_log->LOG("当前座席ID %s 状态为 %s，initial_socket为 %d， sign_socket为 %d", (iter->first).c_str(), state.c_str(), iter->second->m_initial_sock, iter->second->m_signIn_sock);
		center.agent_state_array[iter->second->m_curState]++;
	}
	for(int i = 0; i < static_cast<int>(AGENT_END); i++)
	{
		simu_log->PRINT("当前共有%d个座席状态为：%s", center.agent_state_array[i], center.detail_state_dict[static_cast<DetailState_t>(i)].c_str());
	}
	

	map<int, string>::iterator sock_agentID_iter;
	for(sock_agentID_iter = center.socket_agentID_map.begin(); sock_agentID_iter != center.socket_agentID_map.end(); sock_agentID_iter++)
	{	
		simu_log->LOG("当前socekt %d 对应于座席ID为 %s ", sock_agentID_iter->first, (sock_agentID_iter->second).c_str());
	}
	simu_log->LOG("");

	alarm(alarm_sleep_interval);
	//exit(0);
	//if(enter_alarm_time > 4)
	//{
    //	exit(0);
    //}
}
int main(int argc, char* argv[])
{
	//系统的日志文件
	//unlink("simulation.log");
/*	if(argc == 1)
	{
		if(strcmp(argv[1], "-debug") == 0)
		{
#define _DEBUG true
		}
	}
	
#ifndef _DEBUG 
#define _DEBUG 	false
#endif
*/
	bool _DEBUG = false;
	if(argc == 2)
	{
		if(strcmp(argv[1], "-d") == 0)
			_DEBUG = true;
	}
	
	try
	{

		simu_log = new CLOG("simulation.log", _DEBUG);
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
	int listenWeb = create_connection_to_web(conf->webPort);

	if(listenWeb < 0)
	{
		simu_log->ERROR("web socket错误");
		return 0;
	}

	center.socket_Not_In_Epoll.push(listenWeb);

	if(create_agents(_DEBUG) < 0)
	{
		simu_log->ERROR("初始化座席失败");
		return -1;
	}
	
	struct epoll_event ev,events[MAX_EVENTS];
	
	if(signal(SIGALRM, agentReportAlarm) == SIG_ERR)
	{
		simu_log->ERROR("创建alarm失败");
	}
	alarm(10);
	simu_log->LOG("创建alarm成功");
	//int agent_report_cnt = 0;
	char buf[2500];
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

		int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 20);
		
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
			//ev = events[n];
			
			if(events[n].events & EPOLLERR)
			{
				if(errno != EALREADY && errno != EINPROGRESS)
				{
					simu_log->ERROR("套接字 %d 错误, 错误原因 %s ", events[n].data.fd,strerror(errno));
					close_sock_and_erase(events[n].data.fd);
				}
					
			}
			else if(events[n].events & EPOLLHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",events[n].data.fd);
				close_sock_and_erase(events[n].data.fd);
			}

			else if(events[n].events & EPOLLIN)
			{
				simu_log->LOG("socekt %d 可读", events[n].data.fd);
				if(events[n].data.fd == listenWeb)
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
					simu_log->LOG("是座席的消息");
					//char buf[2500];
					int ret = recv(events[n].data.fd, buf, sizeof(buf) - 1, 0);
					if(ret == 0) 
					{
						simu_log->LOG("socket %d 关闭", events[n].data.fd);
						close_sock_and_erase(events[n].data.fd);
					}
					else if(ret < 0)
					{
						simu_log->ERROR("套接字 %d 接收错误，错误原因 %s",events[n].data.fd, strerror(errno));
					}
					else
					{

						//buf[ret] = '\0';
						//char ret_ch[2500];
						//int ret_pos = 0;
						string msg;
						int pre_pos = 0;
						for(int i = 0; i < ret; i++)
						{
							msg.push_back(buf[i]);
												
						}
						
						/*if(ret < sizeof(buf))
						{
							;
						}
						else
						{
							while((ret = recv(events[n].data.fd, buf, sizeof(buf),0)) > 0)
							{
								buf[ret - 1] = '\0';
								msg = msg + string(buf);
							}

						}*/
						simu_log->LOG("收到来自socket%d的消息,长度为%d %s", events[n].data.fd, msg.size(), msg.c_str());
						handle_message(events[n].data.fd, msg);
					}
					/*if(events[n].events & EPOLLOUT)
					{
						send_message(events[n].data.fd);

					}*/
				}
			}
			else if(center.ready_to_send[events[n].data.fd] && (events[n].events & EPOLLOUT))
			{
				//simu_log->LOG("向socket %d发送消息");
				send_message(events[n].data.fd);
			}
			else if(events[n].events & EPOLLRDHUP)
			{
				simu_log->LOG("套接字 %d 对方关闭连接",events[n].data.fd);
				close_sock_and_erase(events[n].data.fd);
			}
			
		}
	}
}
