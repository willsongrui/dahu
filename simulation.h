#include <string>
#ifndef __simulation__
#define __simulation__
#include "simu_def.h"



//int sendAgentReport();
int handle_message(int sockFd, std::string message);
int send_message(int sockFd);
int non_block_connect(std::string ip, int port);
int load_config(const std::string&);
int close_sock_in_epoll(int sockFd);
int create_agents(bool is_debug);
int create_connection_to_web(int);
int setnonblocking(int fd);
int close_sock_and_erase(int sockFd);
int create_connection_to_cti(std::string ip, int port, CAgent* agent);
std::string add_int_to_string(std::string& base, int n);
int add_to_epoll(int agentfd);
int handle_web_message(int sockFd, std::string&);
//CAgent* find_agent(const std::string&);
CAgent* find_agent(int sockFd);
std::string find_agentID(int sockFd);
int msgSplit(std::vector<std::string>& vec, const std::string& message, std::string& endFlag);
#endif

