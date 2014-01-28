#include <string>
#include <map>
#include <queue>
#include <sys/time.h>
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
	SignOut,Try2Initial,Initial,Try2SignIn,SignIn,Try2Idle,Idle,Try2Busy,Busy,Ringing,Calling,AfterCall
}


class CLOG
{
public:
	CLOG(string file);
	void ERROR();
	void LOG();
	int logFs;
	struct timeval tv;
	struct timezone tz;
private:
	char m_asctime[32];
	char* getTime()
	{
		struct tm *ptr;
		time_t t;
		time(&t);
		ptr = localtime(&t);
		strftime(m_asctime, 100, "%H:%M:%S",ptr);
		return m_asctime;

	}
};

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
	string webSocket;			//IPC名
	string ctiIP;		
	int ctiPort;
	int agentNum;
	string agentID;
	string logFile;
	string vccID;
	string deviceID;
	CConf();
	int validate();
};

class CAgent
{
public:

	//处理来自CTI的与座席相关的消息
	int handle_message();

	//将待发送的消息发送给CTI（待发送的消息保存在队列中）
	int send_message();

	//更新座席的状态，支持两种模式，默认的easy模式不检查状态转移的合法性，而严格状态则会检查并告警
	int setStatus(AgentState,bool easy_mode = true );		
	
	//发送request请求
	int initial();
	int signOut();
	int signIn();
	int setIdle();
	int setBusy();
	
	//每个座席都有一个唯一的日志文件，根据等级不同分为LOG和ERROR
	CLOG* m_log;
	int LOG();
	int ERROR();						 
    int clearLog();
	
	string m_agentID; 				 //cti分配的座席ID
	string m_deviceID;				 //cti分配的设备ID
	
	string m_simuID;					 //在配置文件中的ID
	

	//用于initial的socket相关
	int m_initial_sock;		    	 //initial_socket句柄
	int m_initial_sock_state;          //initial_socket 状态(-1:disconnected;0:connected)
	string m_initial_ip;
	int m_initial_port;

	//用于signIn的socket相关
	string m_sign_ip;
	int m_sign_port;
	int m_sign_sock;
	int m_sign_sock_state;			//(-1:disconnected;0:connected)

	int m_totalCall;     
	int m_successCall;
	
	AgentState m_curState;
	AgentState m_preState;
	
	
	int m_statusAfterCall;		//电话接通后所处的状态：0:后续处理 1：自动空闲 2：自动置忙 3：时间间隔后自动空闲 4：时间间隔后自动置忙
	int m_autoStatusInteval;				//后处理后状态自动切换时间间隔，idleStatus为3或者4时有效后处理后状态自动切换时间间隔，idleStatus为3或者4时有效
	
	string m_timeStamp; 		//在座席登录后系统分配的时间戳
	string m_sessionID;			
	CAgent();
	~CAgent();

	queue <string> m_msgToSend;   //待发送消息
	queue <string> m_msgRecieved;
};

class CCenter
{
public:
	int initial_sock;
	int initial_sock_state; //0:disconnected    1:connected
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


