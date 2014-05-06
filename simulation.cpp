#include "simulation.h"
#include "simu_def.h"
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/epoll.h>
using namespace std;


extern CCenter center;
//extern map <int,string> center.socket_agentID_map;
//extern map <string,CAgent*> center.agentID_agent_map;
extern int epollfd;
extern int write_epollfd;
extern CLOG* simu_log;
extern CConf* conf;
//extern queue<int> center.socket_Not_In_Epoll;

/*
int create_sock_non_blocking(string ip, int port)
{
	int sockFd = socket(AF_INET,SOCK_STREAM,0);
	if(sockFd < 0)
	{
		simu_log->
	}
}*/
int create_connection_to_cti(string ip, int port, CAgent* agent)
{
	agent->log()->LOG("进入%s中", "create_connection_to_cti");
	if((agent==NULL)||(agent->log()==NULL))
	{
		simu_log->ERROR("create_connectino_to_cti时传入指针为空");
		return -1;
	}
	int sockFd = socket(AF_INET,SOCK_STREAM,0);
	if(sockFd < 0)
	{
		agent->log()->ERROR("创建cti套接字失败,失败原因: %s",strerror(errno));
		return -1;
	}
	if(setnonblocking(sockFd) < 0)
	{
		agent->log()->ERROR("create_connection_to_cti时设置非阻塞模式失败");
		return -1;
	}
	agent->log()->LOG("创建cti非阻塞套接字%d成功", sockFd);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	int ret = connect(sockFd,(struct sockaddr*)(&addr), sizeof(addr));
	if((ret < 0)&&(errno!=EINPROGRESS))
	{	
		agent->log()->ERROR("试图connect时失败，错误原因：%s", strerror(errno));
		return -1;
	}
	agent->log()->LOG("试图连接CTI服务器，socket为%d", sockFd);
	center.ready_to_send[sockFd] = false;	
	return sockFd;

}
int add_to_epoll(int agentfd)
{
	center.socket_Not_In_Epoll.push(agentfd);
	return 0;
}
string add_int_to_string(string& base, int n)
{
	//simu_log->LOG(base.c_str());

	if(base.length()<5)
	 	return "WRONG";
	string low = base.substr(base.length()-5);
	int low_value = atoi(low.c_str()) + n;
	if(low_value > 99999)
		return "WRONG"; 
	string ret  = base.substr(0, base.length()-5);
	char tmp[6];
	snprintf(tmp, sizeof(tmp), "%d", low_value);
	ret = ret + string(tmp);
	return ret;
}	
int create_agents(bool is_debug)
{
	/*if(conf==NULL)
	{
		simu_log->ERROR("当试图create_agents时conf实例还没有加载成功");
		return -1;
	}*/
	int _agent_num = conf->agentNum;
	for(int i=0; i < _agent_num; i++)
	{
		CAgent* agent = new CAgent(is_debug);
		agent->m_initial_IP = conf->ctiIP;
		agent->m_initial_Port = conf->ctiPort;
		snprintf(agent->m_agentID, sizeof(agent->m_agentID), "%s", add_int_to_string(conf->agentID, i).c_str());
		
		//agent->m_agentID = add_int_to_string(conf.agentID, i);
		snprintf(agent->m_deviceID, sizeof(agent->m_deviceID), "%s", add_int_to_string(conf->deviceID, i).c_str());
		agent->m_ready = conf->ready;
		agent->m_is_sign_in = false;
		snprintf(agent->m_passwd, sizeof(agent->m_passwd), "%s", conf->passwd.c_str());
		snprintf(agent->m_vccID, sizeof(agent->m_vccID), "%s", conf->vccID.c_str());
		snprintf(agent->m_taskID, sizeof(agent->m_taskID), "%s", conf->taskID.c_str());
		agent->log()->LOG("座席初始化成功，agentID为%s，deviceID为%s，initialIP为%s,initialPORT为%d", agent->m_agentID, agent->m_deviceID,agent->m_initial_IP.c_str(), agent->m_initial_Port);
		
		int agentfd = create_connection_to_cti(agent->m_initial_IP, agent->m_initial_Port, agent);
		if(agentfd < 0)
		{
			agent->log()->ERROR("试图create_connection_to_cti时失败");
			return -1;
		}
		if(add_to_epoll(agentfd) < 0)
		{
			agent->log()->ERROR("试图加入epoll时失败");
			return -1;
		}

		agent->m_initial_sock = agentfd;
		agent->m_connected = false;

		//若新登录的座席存在之前重名的，则将其在map中删除
		center.socket_agentID_map.erase(agentfd);
		center.agentID_agent_map.erase(string(agent->m_agentID));
		
		simu_log->LOG("将initial (%d, %s) 加入到socket_agentID_map中", agentfd, agent->m_agentID);
		center.socket_agentID_map.insert(make_pair(agentfd, string(agent->m_agentID)));
		center.agentID_agent_map.insert(make_pair(string(agent->m_agentID), agent));
		
		agent->initial();
		/*if (i%2 == 0)
		{
			sleep(1);
		}*/
	}
	return 0;
}

int load_config(const string& confFile)
{
	int fs = open(confFile.c_str(),O_RDONLY);
	if(fs < 0)
	{
		simu_log->ERROR("打开配置文件 %s 出错,错误原因: %s", confFile.c_str(), strerror(errno));
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
	catch (parse_error e)
	{
		simu_log->ERROR("配置文件格式错误");
		return -1;
	}
	xml_node<>* root = NULL;
	xml_node<>* ctiIP = NULL;
	xml_node<>* ctiPort = NULL;
	xml_node<>* vccID = NULL;
	xml_node<>* agentNum = NULL;
	xml_node<>* agentID = NULL;
	xml_node<>* webPort = NULL;
	xml_node<>* ready = NULL;
	xml_node<>* passwd = NULL;
	xml_node<>* deviceID = NULL;
	xml_node<>* taskID = NULL;
	root = doc.first_node();
	if(!root)
	{
		simu_log->ERROR("配置文件没有根节点");
		return -1;
	}

	
	
	webPort = root->first_node("webPort");
	ctiIP = root->first_node("ctiIP");
	ctiPort = root->first_node("ctiPort");
	vccID = root->first_node("vccID");
	agentNum = root->first_node("agentNum");
	agentID = root->first_node("agentID");
	ready = root->first_node("ready");	
	passwd = root->first_node("passwd");
	deviceID = root->first_node("deviceID");
	taskID = root->first_node("taskID");
	if(!(root && passwd && ready && webPort &&deviceID && ctiIP && ctiPort && vccID && taskID && agentNum && agentID))
	{
		simu_log->ERROR("配置文件节点不全");
		return -1;	
	}
	
	
	conf->webPort = atoi(webPort->value());
	conf->ctiIP = string(ctiIP->value());
	conf->ctiPort = atoi(ctiPort->value());
	conf->vccID = string(vccID->value());
	conf->agentNum = atoi(agentNum->value());
	conf->agentID = string(agentID->value());
	conf->ready = atoi(ready->value());
	conf->passwd = string(passwd->value());
	conf->deviceID = string(deviceID->value());
	conf->taskID = string(taskID->value());

	simu_log->LOG("配置文件读取成功");
	
	//_delete root;
	//_delete webPort;
	//_delete ctiIP;
	//_delete ctiPort;
	//_delete vccID;
	//_delete agentNum;
	//_delete agentID;
	//_delete passwd;
	//_delete ready;

	return 0;
}

int all_initial()
{

	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if((iter->second)->initial() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席initial失败",iter->first.c_str());
			return -1;
		}
	}
	return 0;	
}

int all_signIn()
{
	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if((iter->second)->signIn() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席signIn失败",iter->first.c_str());
			return -1;
		}
	}
	return 0;

}

int all_setIdle()
{
	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if((iter->second)->setIdle() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席setIdle失败",iter->first.c_str());
			return -1;
		}
	}
	return 0;

}

int all_setBusy()
{
	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if((iter->second)->setBusy() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席setBusy失败",iter->first.c_str());
			return -1;
		}
	}
	return 0;

}

int all_signOut()
{
	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		if((iter->second)->signOut() < 0)
		{
			simu_log->ERROR("ID为 %s 的座席signOut失败",iter->first.c_str());
			return -1;
		}
	}
	return 0;

}

int all_report(int sockFd)
{/*
	string report_message = string(conf.agentNum);
	map <string, CAgent*>::iterator iter;
	for(iter = center.agentID_agent_map.begin(); iter != center.agentID_agent_map.end(); iter++)
	{
		message = message + " " + string(iter->sockState) + " " + string(iter->curState) + " " + string(iter->successCall) + " " + string(iter->totalCall);
	}
	map <int, queue<string> >it = center.webSocket.find(sockFd);
	if(it == center.webSocket.end())
	{
		simu_log->ERROR("监听服务器套接字 %d 没有找到",sockFd);
		return -1;
	}
	(it->second).push(message);
	simu_log->LOG("向监听服务器套接字 %d 发送report，内容为%s",sockFd,message.c_str());
	return 0;
	*/
}

int handle_web_message(int sockFd, string& message)
{

	//simu_log->LOG("收到服务器套接字 %d 的消息，内容为%s",sockFd,message.c_str());
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

void strip(string& str)
{
	int x = 0;
	int y = str.size();
	for(int i=0; i<str.size(); i++)
	{
		if(str[i] == ' ')
			x = x + 1;
		else
			break;
	}
	for(int j=str.size(); j > x; j--)
	{
		if(str[j] == ' ')
			y = y - 1;
		else
			break;
	}
	str = str.substr(x, y-x);


}

void strip_vec(vector<string>& vec)
{
	for(int i=0; i < vec.size(); i++)
	{
		strip(vec[i]);
	}
}
int msgSplit(vector<string>& vec, const string& message, string& endFlag)
{
	int has_imcomplete_message = 0;
	int message_len = message.size();
	string cur;
	cur.reserve(message_len);
	for(int i = 0; i < message_len; i++)
	{
		if(message[i] != '\0')
		{
			cur.push_back(message[i]);
		}
		else
		{
			vec.push_back(cur);
			cur.clear();
		}	
	}
	if(message[message_len - 1] != '\0')
	{
		has_imcomplete_message = 1;
		vec.push_back(cur);
	}
	return has_imcomplete_message;

}
/*
int msgSplit(vector<string>& vec, const string& message, string& endFlag)
{
	int has_imcomplete_message = 0;
	//simu_log->LOG("进入%s中, 长度为%d, 消息为%s", "msgSplit", message.length(), message.c_str());	
	
	for()


	int len = endFlag.size();
	int pos = 1;
	int pre = 0;
	pos = message.find(endFlag);
	string remain;
	while(pos != string::npos)
	{
		vec.push_back(message.substr(pre, pos+len-pre));
		
		pre = pos + len;
		pos = message.find(endFlag, pre);
	}
	if(pre >= message.length() || pre == string::npos)
	{
		has_imcomplete_message = 0;
	}
	else
	{
		has_imcomplete_message = 1;
		remain = message.substr(pre);
		simu_log->LOG("要加入remain的pre为%d, ", pre);
		vec.push_back(remain);
	}
	strip_vec(vec);
	return has_imcomplete_message;
}
*/
int handle_message(int sockFd, string message)
{
	//simu_log->LOG("进入%s中,收到%s", "handle_message", message.c_str());
	//simu_log->PRINT("handle_message");
	int n;
	int ret = 0;
	vector<string> msg;
	if(center.webSocket.find(sockFd)!=center.webSocket.end())		//webSocket是接收web命令的套接字
	{
		simu_log->LOG("收到的是webSocket消息");
		string endFlag("</webmsg>");
		msgSplit(msg, message, endFlag);
		for(int i=0; i < msg.size(); i++)
		{	
			n = handle_web_message(sockFd, msg[i]);
			if(n < 0)
			{
				simu_log -> ERROR("处理web_message时失败:%s",msg[i].c_str());
				ret = -1;
			}
		}
	}
	else
	{
		CAgent* agent = find_agent(sockFd);
		if(!agent)
		{
			simu_log->LOG("no agent has fd %d",sockFd);
			return -1;
		}
		else
		{
			if(agent->m_has_initial_port == false)
			{
				struct sockaddr_in sin;
				memset(&sin, 0, sizeof(sin));
				int sin_size = sizeof(sin);
				if(getsockname(sockFd, (struct sockaddr*)&sin, (socklen_t*)&sin_size) ==0)
				{
					agent->m_has_initial_port = true;
					agent->m_src_initial_port = htons(sin.sin_port);
					agent->log()->LOG("座席initial端口号为%d", agent->m_src_initial_port);
				}
			}
			else if(agent->m_has_signIn_port == false)
			{
				struct sockaddr_in sin;
				memset(&sin, 0, sizeof(sin));
				int sin_size = sizeof(sin);
				if(getsockname(sockFd, (struct sockaddr*)&sin, (socklen_t*)&sin_size) ==0)
				{
					agent->m_has_signIn_port = true;
					agent->m_src_signIn_port = htons(sin.sin_port);
					agent->log()->LOG("座席signIn端口号为%d", agent->m_src_signIn_port);
				}
			}
			//simu_log->LOG("收到的是agent消息");
			if(agent->m_remain_msg.length() != 0)
			{
				agent->log()->LOG("座席有上次未完全解析的报文，为%s", agent->m_remain_msg.c_str());
				message = agent->m_remain_msg + message;
				agent->m_remain_msg.clear();
			}

			agent->log()->LOG("从handle_message入口处收到消息%s", message.c_str());
			string endFlag("</acpMessage>");
			int _ret = msgSplit(msg, message, endFlag);
			//int _ret = 0;
			if(_ret < 0)
			{
				agent->log()->ERROR("msgSplit失败");
				return -1;
			}
			if(_ret == 1)
			{
				agent->log()->LOG("simulation handle_message 函数收到消息中有不完整的片段, 共%d段报文", msg.size());
				agent->m_remain_msg = msg[msg.size()-1];
				for(int i = 0; i < msg.size(); i++)
				{
					agent->log()->LOG("解析的第%d段：%s", i+1, msg[i].c_str());
				}
			}
						
			int handle_size = _ret == 1 ? msg.size()-1 : msg.size();
			
			for(int i=0; i < handle_size; i++)
			{
				n = agent->handle_message(msg[i], sockFd);
				//agent->log()->LOG("545 handle_message %s ", msg[i].c_str());
				if(n < 0)
				{
					//simu_log -> ERROR("处理agent_message时失败:%s",msg[i].c_str());
					ret = -1;
				}

			}
		}
	}
	if(ret == 0)
	{
		simu_log->LOG("成功handle_message");
	}
	else
	{
		simu_log->ERROR("handle_message失败");
	}
	return ret;
}


int send_message(int sockFd)
{
	center.ready_to_send[sockFd] = false;
	map <int,queue<string> >::iterator it;
	it = center.webSocket.find(sockFd);

	if(it != center.webSocket.end())			//此套接字是webSocket
	{
		while((it->second).empty()!=false)
		{
			if(send(sockFd,(it->second).front().c_str(), sizeof((it->second).front().c_str()), 0) < 0)
			{
				simu_log->ERROR("套接字 %d 向web发送消息 %s 错误,错误原因是 : %s",sockFd,(it->second).front().c_str(),strerror(errno));
			}
			simu_log->LOG("成功发送web socket消息");
			(it->second).pop();
		}
	}
	else
	{
		CAgent* agent = find_agent(sockFd);
		if(!agent)
		{
			simu_log->ERROR("套接字 %d 既不是座席也不是web",sockFd);
			return -1;
		}
		else
		{
			if(agent->send_message(sockFd) < 0)
			{
				simu_log->ERROR("座席 %s (套接字 %d)发送消息错误",agent->m_agentID,sockFd);
				return -1;
			}
			//agent->log()->LOG("发送消息");
		}
	}
	return 0;
}


int setnonblocking(int sockFd)
{
	//simu_log->INFO("进入%s中", "setnonblocking");
	if(sockFd <= 0)
	{
		simu_log->ERROR("待设置为nonblocking的句柄为负");
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
	//simu_log->INFO("离开%s中", "setnonblocking");
	return 0; 
}
/*
CAgent* find_agent(const string& agentID)
{
	
	map <string,CAgent*>::iterator iter;
	iter = center.agentID_agent_map.find(agentID); 
	if(iter !=center.agentID_agent_map.end())
		return iter->second;
	else
		return NULL;

}*/

CAgent* find_agent(int sockFd)
{	
	string agentID = find_agentID(sockFd);
	if("NONEXIST" == agentID)
	{
		return NULL;
	}
	map <string,CAgent*>::iterator iter;
	iter = center.agentID_agent_map.find(agentID); 
	
	if(iter !=center.agentID_agent_map.end())
		return iter->second;
	else
		return NULL;

}
string find_agentID(int sockFd)
{
	map <int,string>::iterator iter;
	iter = center.socket_agentID_map.find(sockFd);
	if(iter == center.socket_agentID_map.end())
	{
		simu_log->ERROR("查找socket:%d时失败", sockFd);
		map <int,string>::iterator it;
		string sockets;

		for(it = center.socket_agentID_map.begin(); it!=center.socket_agentID_map.end(); it++)
		{
			int cur_sock = it->first;
			char temp[20];
			sprintf(temp, "%d ", cur_sock);
			sockets = sockets + string(temp);
		}
		simu_log->ERROR("当前socket_agentID_map中socket只含有%s", sockets.c_str());
		string NOTFOUND("NONEXIST");
		return NOTFOUND;
	}
	
	return iter->second;

}

string describe_detail_state(DetailState_t)
{
	map<DetailState_t, string> dictionary;
}





int close_sock_and_erase(int sockFd)
{
	//simu_log->INFO("试图close_sock_and_erase%d", sockFd);
	
	if(epoll_ctl(epollfd, EPOLL_CTL_DEL, sockFd, NULL) < 0)
	{
		
		simu_log->ERROR("%s while deleting fd %d",strerror(errno),sockFd);
		//return -1;
	}
	if(epoll_ctl(write_epollfd, EPOLL_CTL_DEL, sockFd, NULL) < 0)
	{
		
		simu_log->ERROR("%s while deleting fd %d",strerror(errno),sockFd);
		//return -1;
	}
	shutdown(sockFd, SHUT_RDWR);
	
	map <int, queue<string> >::iterator iter;
	iter = center.webSocket.find(sockFd);
	if(iter != center.webSocket.end())
	{
		//sockFd是监听web套接字
		center.webSocket.erase(iter);
		return 0;
	}
	/*string agentID = find_agentID(sockFd);
	AgentID_t agentID;
	strcpy(agentID, str_agentID.c_str());
	if(strcmp(agentID, "NONEXIST")==0)
	{
		simu_log->ERROR("当试图在寻找sockFd为%d的agentID时失败",sockFd);
		return -1;
	}*/
	
	CAgent* agent = find_agent(sockFd);
	simu_log->LOG("将socket:%d从socket_agentID_map中删掉", sockFd);
	center.socket_agentID_map.erase(sockFd);
	
	if(agent == NULL)
	{
		simu_log->ERROR("当试图在寻找sockFd为%d的agent时失败",sockFd);
		return -1;	
	}
	int type = agent->find_sock_type(sockFd);
	if(type == 0)
	{
		agent->log()->LOG("initial的套接字关闭了");
		agent->m_initial_sock = -999;
	}
	else if(type == 1)
	{
		agent->log()->LOG("signIn的套接字关闭了");
		agent->m_signIn_sock = -999;
		agent->m_connected = false;
	}
	else
	{
		agent->log()->ERROR("Agent的socket type设置错了");
		return -1;
	}

	/*
	if(agent->agentState == AG_NULL)
	{
		agent->set_initial_sock(-1);
		agent->log()->LOG("Initial 套接字关闭");
	}
	else
	{
		agent->set_sign_sock(-1);
		agent->log()->LOG("正常登录座席的套接字关闭");
		agent->set_agentState(AG_NULL);
	}
	*/
	return 0;
}

int create_connection_to_web(int Port)
{

	//simu_log->LOG("进入create_connection_to_web");
	int sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd < 0)
	{
		simu_log->ERROR("创建web套接字失败,失败原因: %s", strerror(errno));
		return -1;
	}
	int flags = fcntl(sockFd, F_GETFL);
	if(flags < 0)
	{
		simu_log->ERROR("得到web套接字flag失败,失败原因: %s", strerror(errno));
		return -1;	
	}
	if(fcntl(sockFd, F_SETFL, flags|O_NONBLOCK) < 0)
	{
		simu_log->ERROR("设置web套接字为非阻塞模式失败,失败原因: %s", strerror(errno));
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(Port);
	if(bind(sockFd, (struct sockaddr*)(&addr), sizeof(addr)) < 0)
	{
		simu_log->ERROR("绑定sockect时失败");
		return -1;
	}
	listen(sockFd,1000);
	simu_log->LOG("正在监听%d，等待web端连接");
	center.ready_to_send[sockFd] = false;
	return sockFd;
	
}
























