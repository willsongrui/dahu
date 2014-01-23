#include <string>
#include <map>
#include <queue>
using namespace std;


/* 
0: LogOut  (没有签入）
1: Initial (正在签入,连接服务器，得到CTI服务器地址)
2: SignIn  (已经签入但是未示闲）
3: Idle	   (准备好）
4: Ringing (正在振铃)
5: Calling（正在通话）
*/ 
enum AgentState
{
	LogOut,Initial,SignIn,Idle,Ringing,Calling,
}




class CMessage
{
public:
	string initialRequestMsg;
	string signInRequstMsg;
	string setIdleRequestMsg;
	CMessage();

};

//CAgent 
class CConf
{
public:
	string ipc;			//IPC名
	string ctiIp;		
	int ctiPort;
	int agentNum;
	int agentId;
	string logFile;
	CConf();
};

class CAgent
{
public:
	int logIn();
	int logOut();
	int signIn();
	int setIdle();
	int logOut();
	int log();
    int clearLog();
	string agentID; 
	CMessage message;		 //CTI request消息
	int sock;		    	 //socket句柄
	int sockState;           //socket 状态
	int totalCall;     
	int successCall;
	int logFd;          	 //日志文件句柄
	AgentState curState;
	AgentState preState;
	
	int handle_message();
	int send_message();
	string timeStamp; 		 //在座席登录后系统分配的时间戳

	CAgent();
	~CAgent();

	queue <string> msgToSend;  //待发送消息
	queue <string> msgRecieved;
};

class CCenter
{
public:
	map < int, queue <string> > webSocket;
	map <string,CAgent> agentID_CAgent_Map;
	int agentNum;
	vector <string> agentID;
	int totalCall;
	int successCall;
	string ctiIp;
	short ctiPort;
	string ipc;// 连接web服务器的socket
	string CTI;

	CCenter();
	~CCenter();
};


