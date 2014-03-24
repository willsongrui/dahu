#ifndef __CAGENT__
#define __CAGENT__
#include <string>

class CAgent
{
public:
	//处理来自CTI的与座席相关的消息
	int handle_message(const std::string&);
	//将待发送的消息发送给CTI（待发送的消息保存在队列中）
	int send_message();
	//更新座席的状态，支持两种模式，默认的easy模式不检查状态转移的合法性，而严格状态则会检查并告警
	int setStatus(DetailState_t, bool easy_mode = true );		
	//发送request请求
	int initial();
	int signOut();
	int signIn();
	int setIdle();
	int setBusy();
	int sendHeartBeat();
	int agentReport();
	
	//每个座席都有一个唯一的日志文件，根据等级不同分为LOG和ERROR
	CLOG* log();
	AgentState_t m_agentStatus;
	PhoneState_t m_phoneStatus;
	bool isSignIn;
	AgentPasswd_t m_passwd;

	int sign_sock();
	int initial_sock();
	void set_sign_sock(int);
	void set_initial_sock)(int);
	
	void CAcpParse::BuildGeneralConf(ACPGeneralConfEvent_t &generalConf,xml_node<>* hBody);
	int setAgentStatus(DetailState_t);

private:
	bool m_isSignIn;
	ACPEvent_t m_acpEvent;
	static std::set<std::string> allowed_cmd;
	static std::map <std::string,EventType_t> eventTypeMap;
	std::string m_ready;
	CLOG* m_log;
	
	AgentID_t m_destAgentID;
	VccID_t m_vccID;
	AgentID_T m_agentID;
	
    SessionID_t m_sessionID;
	DeviceID_t m_deviceID;				 //cti分配的设备ID
	TimeStamp_t m_timeStamp; 		//在座席登录后系统分配的时间戳
	
	int m_sign_sock;                       //初始为-1
	int m_initial_sock;		    	 //initial_socket句柄
	int	m_lMsgReceived;				//描述接收到的消息是否成功

	
	
	
	std::string m_initial_IP;
	int m_initial_Port;

	//用于signIn的socket相关
	std::string m_signIn_IP;
	int m_signIn_Port;

	int m_totalCall;     
	int m_successCall;
	
	DetailState_t m_curState;
	DetailState_t m_preState;
	
	
	int m_statusAfterCall;		//电话接通后所处的状态：0:后续处理 1：自动空闲 2：自动置忙 3：时间间隔后自动空闲 4：时间间隔后自动置忙
	int m_autoStatusInteval;				//后处理后状态自动切换时间间隔，idleStatus为3或者4时有效后处理后状态自动切换时间间隔，idleStatus为3或者4时有效
				
	CAgent();
	~CAgent();

	std::queue <std::string> m_msgToSend;   //待发送消息
	
};
#endif