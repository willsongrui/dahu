#include <queue>
#include <map>
using namespace std;
int handle_message(int sockFd,string message);
int send_message(int sockFd);
int non_block_connect(string ip,int port);
int load_config(string);
int close_sock_in_epoll(int sockFd);

int create_agents();


