#ifndef __simulation__
#define __simulation__

int handle_message(int sockFd,std::string message);
int send_message(int sockFd);
int non_block_connect(std::string ip,int port);
int load_config(const std::string&);
int close_sock_in_epoll(int sockFd);
int create_agents();

#endif

