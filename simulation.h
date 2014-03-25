#ifndef __simulation__
#define __simulation__
#include <string>
int handle_message(int sockFd,std::string message);
int send_message(int sockFd);
int non_block_connect(std::string ip, int port);
int load_config(const std::string&);
int close_sock_in_epoll(int sockFd);
int create_agents();
int create_connection_to_web(int);
int setnonblocking(int fd);
int close_sock_and_erase(int sockFd);

#endif

