class CAgent
{
public:
	//处理来自CTI的与座席相关的消息
	int handle_message();
	//将待发送的消息发送给CTI（待发送的消息保存在队列中）
	int send_message();
	//更新座席的状态，支持两种模式，默认的easy模式不检查状态转移的合法性，而严格状态则会检查并告警
	int setStatus(DetailState_t, bool easy_mode = true );		
	CMessage message;
	//发送request请求
	int initial();
	int signOut();
	int signIn();
	int setIdle();
	int setBusy();
	
	//每个座席都有一个唯一的日志文件，根据等级不同分为LOG和ERROR
	CLOG* log();
	
private:
	string m_ready;
	CLOG* m_log;
	
	AgentID_t m_destAgentID;
	VccID_t m_vccID;
	AgentID_T m_agentID;
	Passwd_t m_passwd;
    SessionID_t m_sessionID;
	AgentState_t m_agentStatus;
	PhoneState_t m_phoneStatus;
	DeviceID_t m_deviceID;				 //cti分配的设备ID
	TimeStamp_t m_timeStamp; 		//在座席登录后系统分配的时间戳

	//用于initial的socket相关
	int m_initial_sock;		    	 //initial_socket句柄
	int m_initial_sock_state;          //initial_socket 状态(-1:disconnected;0:connected)
	
	string m_initial_IP;
	uint16_t m_initial_Port;

	//用于signIn的socket相关
	string m_signIn_IP;
	uint16_t m_signIn_Port;
	int m_sign_sock;
	int m_sign_sock_state;			//(-1:disconnected;0:connected)

	int m_totalCall;     
	int m_successCall;
	
	DetailState_t m_curState;
	DetailState_t m_preState;
	
	
	int m_statusAfterCall;		//电话接通后所处的状态：0:后续处理 1：自动空闲 2：自动置忙 3：时间间隔后自动空闲 4：时间间隔后自动置忙
	int m_autoStatusInteval;				//后处理后状态自动切换时间间隔，idleStatus为3或者4时有效后处理后状态自动切换时间间隔，idleStatus为3或者4时有效
				
	CAgent();
	~CAgent();

	queue <string> m_msgToSend;   //待发送消息
	queue <string> m_msgRecieved;
};
