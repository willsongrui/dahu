#ifndef __simu_def__
#define __simu_def__
#include <string>
#include <map>
#include <queue>
#include <sys/time.h>
#include <errno.h>
#include <set>
#include "log.h"
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"

using namespace rapidxml;

typedef char 			VccID_t[32];
typedef char 			AgentID_t[32];
typedef char 			AgentPasswd_t[32];
typedef char 			SessionID_t[64];
typedef char 			TimeStamp_t[64];
typedef char 			DeviceID_t[64];
typedef char            DATA_t[1024];
typedef char            ServerIP_t[49];
typedef char            Description_t[256];


const int MSG_FAILURE = -1;
const int MSG_SUCCESS = 0;
const char LoadBalance[10] = "on";
typedef enum  
{
    CS_IDLE = 0,
    CS_ORIGATED,
    CS_INITIATED ,
    CS_QUEUED ,
    CS_DELIVERED ,
    CS_ALERTING,
    CS_CONNECTED,
    CS_CONFERENCED,
	CS_HELD
} PhoneState_t;

typedef enum
{
	AG_NULL = 0,
	AG_NOT_READY = 1,
	AG_READY = 2,
	AG_BUSY = 3,
	AG_WORKING_AFTER_CALL = 4,

}AgentState_t;
typedef enum 
{
	AGENT_BUSY,
	AGENT_IDLE,
	AGENT_SIGNIN,
	AGENT_SIGNOUT,
	AGENT_RELEASE_SUCCESS,
	AGENT_ANSWERING,
	
}DetailState_t;

typedef struct
{
	PhoneState_t    phonestatus;
    long            legID;
    DeviceID_t      deviceID;
}Device_t;

typedef struct  
{
    int             code;
    Description_t   desc;
} Cause_t;

typedef struct
{
    int          type; 
    VccID_t      vccID; 
    AgentID_t    agentID;
	DeviceID_t   deviceID;
	AgentState_t agentStatus;
	PhoneState_t phoneStatus;
	int          master;//0|1
} Agent_t;

typedef struct
{
    DeviceID_t   callingDevice;
    DeviceID_t   calledDevice;
    DeviceID_t   orgCalledDevice; 
    DeviceID_t   orgCallingDevice; 
    DATA_t       callData;
    int          count;
	Device_t*    device;
}Callinfo_t;


typedef struct
{
	DeviceID_t callID;
	Agent_t    agent;
	Cause_t    cause;
} Parameter_t;


typedef struct
{
    Parameter_t   parameter ;
    DeviceID_t    releaseDevice  ; 
	int           releaseLegID;
    Callinfo_t    callInfo ; 
} ACPReleaseEvent_t;

typedef struct
{
    Parameter_t   parameter ;
    Callinfo_t    callInfo ; 
} ACPCallinfoEvent_t;






typedef struct
{
    Parameter_t   parameter ;
    Agent_t       destAgentInfo ; 
} ACPQueryAgentStatus_t;

typedef struct
{
    Parameter_t   parameter ;
    Agent_t       destAgentInfo ; 
    Callinfo_t    callinfo ; 
} ACPQueryAgentCallInfo_t;


typedef struct
{
    Parameter_t   parameter ;
    DATA_t        callData; 
} ACPGetCallData_t;

typedef struct
{
    Parameter_t   parameter ;
    DeviceID_t    callingNo ; 
    DeviceID_t    calledNo ; 
}ACPHangupCallEvent_t;

typedef struct
{
    Parameter_t   parameter ;
    DeviceID_t    fileName ; 
}ACPRecordInfoEvent_t;

typedef enum
{
    EC_REQUEST          = 0,
    EC_UNSOLICITED      = 1,
    EC_CONFIRMATION     = 2
} EventClass_t;

typedef enum
{
    ACP_UNKNOWN                         = 0,
    ACP_HEART_BEAT                      = 1,

    ACP_Initial_CONF                    = 2,
    ACP_SignIn_CONF,
    ACP_SignOut_CONF,
    ACP_SetBusy_CONF,
    ACP_SetIdle_CONF,
    ACP_AgentReport_CONF,	
    ACP_SetAgentAutoEnterIdle_CONF,
    ACP_AgentEnterIdle_CONF,
    ACP_AgentEnterWrapUp_CONF,
    ACP_ForceIdle_CONF,
    ACP_ForceBusy_CONF,
    ACP_ForceOut_CONF,
    ACP_ReleaseCall_CONF,
    ACP_ReleaseCallByCallID_CONF,
	ACP_SendDTMF_CONF,
    ACP_CallInside_CONF,
    ACP_Consultation_CONF,
    ACP_CallOutside_CONF,
    ACP_SingleStepTransfer_CONF,
    ACP_Transfer_CONF,
    ACP_SetTransfer_CONF,
    ACP_Hold_CONF,
    ACP_RetrieveHold_CONF,
    ACP_Conference_CONF,
    ACP_Alternate_CONF,
    ACP_DoReplyMultimeAndHangUpCall_CONF,
    ACP_Listen_CONF,
    ACP_Intercept_CONF,
    ACP_Insert_CONF,
    ACP_StopInsert_CONF,
    ACP_BeginRecord_CONF,
    ACP_StopRecord_CONF,
	ACP_Bridge_CONF,
    ACP_SetCallData_CONF,
	ACP_GetCallData_CONF,
    ACP_QueryTotalQueueWaitNum_CONF,
    ACP_QueryQueueWaitNum_CONF,
    ACP_QueryAgentStatus_CONF,
    ACP_GetAccessNo_CONF,
    ACP_QueryAgentCallInfo_CONF,
	ACP_SetCTIParam_CONF,

	ACP_OnOrigated                     = 80,
    ACP_OnAnswerRequest,
    ACP_OnAnswerSuccess,
    ACP_OnAnswerFailure,
    ACP_OnUserHangupBeforeResponse,
	ACP_OnRequestRelease,
    ACP_OnReleaseSuccess,
    ACP_OnReleaseFailure,
    ACP_OnLongNoAnswer,
	ACP_OnConsultationBack,
	ACP_OnConsultationSuccess,
	ACP_OnConsultationFailure,
	ACP_OnTransSuccess,
	ACP_OnTransFailure,
	ACP_OnReturnFromIVR,
	ACP_OnReturnFromPhone,
	ACP_OnBridgeSuccess,
	ACP_OnBridgeFailure,
    ACP_OnSingleTransferSuccess,
    ACP_OnSingleTransferFailure,
    ACP_OnCallInsideSuccess,
    ACP_OnCallInsideFailure,
    ACP_OnCallInsideSucTalk,
    ACP_OnCallOutsideSuccess,
    ACP_OnCallOutsideFailure,
    ACP_OnCallOutsideSuccTalk,
    ACP_OnHoldSuccess,
	ACP_OnHoldFailure,
	ACP_OnRetrieveHoldSuccess,
	ACP_OnRetrieveHoldFailure,
	ACP_OnHoldCallRelease,
	ACP_OnConferenceSuccess,
	ACP_OnConferenceFailure,
	ACP_OnConferenceSucTalk,
	ACP_OnHangupCallInConf,
	ACP_OnAlternateSuccess,
	ACP_OnAlternated,
	ACP_OnAlternateFailure,	
	ACP_OnListenSuccess,
	ACP_OnListenFailure,
	ACP_OnInsertSuccess,
	ACP_OnInsertFailure,
	ACP_OnStopInsertSuccess,
	ACP_OnStopInsertFailure,
	ACP_OnInterceptSuccess,
	ACP_OnInterceptFailure,
	ACP_OnForceIdleSuccess,
	ACP_OnForceIdleFailure,
	ACP_OnForceBusySuccess,
	ACP_OnForceBusyFailure,
	ACP_OnForceOutSuccess,
	ACP_OnForceOutFailure,
	ACP_OnBeginRecordSuccess,
	ACP_OnBeginRecordFailure,
	ACP_OnStopRecordSuccess,
	ACP_OnStopRecordFailure,
	ACP_OnForceIdle,
	ACP_OnForceBusy,
	ACP_OnForceOut,
	ACP_OnSignOuted,
	ACP_OnForceRelease,
	ACP_OnAgentAllBusy,
	ACP_OnIntercepted,
	ACP_OnInserted,
	ACP_OnStopListenSuccess,
	ACP_OnStopListenFailure,
	ACP_OnListened,
	ACP_OnSystemIdle,
	ACP_OnQueueReport,
	ACP_OnSystemMessage
} EventType_t;


typedef struct 
{
    SessionID_t    sessionID;
    SessionID_t     serialID;
	int             serviceID;
    TimeStamp_t     timeStamp;
    EventClass_t   eventClass;
    EventType_t    eventType;
} ACPEventHeader_t;

typedef Parameter_t ACPGeneralConfEvent_t;



typedef struct
{
	int             code;
	Description_t   desc;
    ServerIP_t      ip ;
    long            port; 
} ACPInitialConfEvent_t;

typedef struct
{
    Parameter_t   parameter ;
    Callinfo_t    callInfo ; 
} ACPOrigatedEvent_t;

typedef struct
{
    Parameter_t   parameter ;
    DeviceID_t    alertingDevice; 
    Callinfo_t    callInfo ; 
} ACPAnswerRequestEvent_t;

typedef struct
{
	int             idleStatus;  //ºô½Ð½áÊøºó´¦Àí±êÖ¾0£º±£³ÖÔÚAgentWorkingAfterCall×´Ì¬1£º×Ô¶¯½øÈëAgentReady×´Ì¬
	Description_t   groupID;    //
    int             agentType ;//0£ºÆÕÍ¨»°ÎñÔ±1£º°à³¤2£ºÖÊ¼ìÔ±
    int             locked; //0£ºÎ´Ëø¶¨1£ºÒÑËø¶¨
	int             allTimeRecord;  //0£º²»Â¼  1£ºÆÕÍ¨Â¼Òô
	int             deviceType;     //0£º²»Â¼  1£ºÆÕÍ¨Â¼Òô
	Description_t   ctiEvent;       //CTI×Ô¶¯»úºÅ
} ACPAgentParam_t;

typedef struct
{
    Parameter_t      parameter ;
	ACPAgentParam_t  agentParam ;
} ACPSignOutConfEvent_t;


typedef struct 
{
    union
    {
        ACPGeneralConfEvent_t     generalConf;
		ACPInitialConfEvent_t     initialConf;
		ACPSignOutConfEvent_t     signOutcConf;
		ACPQueryAgentStatus_t     queryagentstatus;
		ACPQueryAgentCallInfo_t   queryAgentCallInfo;
		ACPGetCallData_t          getCallData;
    }u;
}ACPConfirmationEvent_t;

typedef Parameter_t ACPEventReportEvent_t;

typedef struct
{
    union
    {
        ACPEventReportEvent_t     generalEventReport;
		ACPOrigatedEvent_t        origatedEventReport;
		ACPAnswerRequestEvent_t   answerRequestEventReport;
		ACPCallinfoEvent_t        callinfoEventReport;
		ACPReleaseEvent_t         releaseEventReport;
 		ACPHangupCallEvent_t      hangupCallEventReport;
		ACPRecordInfoEvent_t      recordInfoEventReport;
   } u;
} ACPUnsolicitedEvent_t;

typedef struct
{
    ACPEventHeader_t eventHeader;
    union
    {
        ACPConfirmationEvent_t    acpConfirmation;
        ACPUnsolicitedEvent_t     acpEventReport;
    } event;
} ACPEvent_t;

typedef struct
{
	SessionID_t    sessionID;
    SessionID_t     serialID;
	int             serviceID;
    TimeStamp_t     timeStamp;
    EventClass_t   eventClass;
    EventType_t    eventType;

} MsgParser_t;


class CConf
{
public:
	CConf();
	int webPort;			
	std::string ctiIP;		
	int ctiPort;
	int agentNum;
	std::string agentID;
	std::string deviceID;
	std::string logFile;
	std::string vccID;
	int ready;
	std::string passwd;
};

typedef struct
{
 //  <N-CTS ip="192.168.2.217" port="24001"/>	CString dn;
	std::string Ip;
	long    port;
	std::string protocol;
}serverInfo;


class CAgent
{
public:
	//处理来自CTI的与座席相关的消息
	int handle_message(std::string&, int sockFd, bool quick=true);
	//将待发送的消息发送给CTI（待发送的消息保存在队列中）
	//int send_message();
	//更新座席的状态，支持两种模式，默认的easy模式不检查状态转移的合法性，而严格状态则会检查并告警
	int setStatus(DetailState_t, bool easy_mode = true );		
	//发送request请求
	int initial();
	int signOut();
	int signIn();
	int setIdle();
	int setBusy();
	int forceBusy();
	int forceIdle();
	int forceOut();
	int releaseCall();
	int send_message(int sockFd);
	std::string getHeader();
	int sendMsg(std::string&, int type);
	int sendMsgEx(std::string&, const char*);
	int sendHeartBeat();
	int agentReport();
	
	int BuildGeneralConf(ACPGeneralConfEvent_t &generalConf, xml_node<>* hBody);
	int BuildAgentInfo(Agent_t &agentInfo,xml_node<>* body);
	int BuildCauseInfo(Cause_t &causeInfo,xml_node<>* body);
	int BuildIntialConf(ACPInitialConfEvent_t &initialConf,xml_node<>* body);
	int BuildCallinfo(Callinfo_t &callInfo, xml_node<>* body);
	int BuildGeneralEventReport(ACPEventReportEvent_t &generalEventReport,xml_node<>* body);
	int BuildAnswerRequestEventReport(ACPEvent_t &msg,xml_node<>* body);
	int BuildHangupCallInfo(ACPHangupCallEvent_t &hangupCallEventReport,xml_node<>* body);
	int BuildReleaseEventReport(ACPEvent_t &msg,xml_node<>* body);
	int BuildRecordInfo(ACPRecordInfoEvent_t &recordInfoEventReport, xml_node<>* body);
	int BuildCallinfoEventReport(ACPEvent_t &msg,xml_node<>* body);
	int msgParse(std::string& msg);
	int find_sock_type(int);




	int setAgentStatus(DetailState_t, bool);
	std::string find_cmd(const std::string&, int);
	int handle_msg();

	std::string remain_msg;
	//每个座席都有一个唯一的日志文件，根据等级不同分为LOG和ERROR
	CLOG* log();
	AgentState_t m_agentStatus;
	PhoneState_t m_phoneStatus;
	//bool m_is_sign_in;
	bool m_connected;
	AgentPasswd_t m_passwd;
	int m_idleStatus;
	int m_agentType;
	int m_locked;
	int m_allTimeRecord;
	int m_deviceType;
	DeviceID_t m_calledDevice;
	DeviceID_t m_orgCalledDevice;
	DeviceID_t m_callingDevice;
	DeviceID_t m_orgCallingDevice;
	bool m_is_sign_in;
	std::string m_initial_IP;
	int m_initial_Port;

	ACPEvent_t m_acpEvent;
	static std::set<std::string> allowed_cmd;
	static std::map <std::string,EventType_t> eventTypeMap;
	int m_ready;
	CLOG* m_log;
	
	AgentID_t m_destAgentID;
	VccID_t m_vccID;
	AgentID_t m_agentID;
	std::string m_vdcode;
    SessionID_t m_sessionID;
	DeviceID_t m_deviceID;				 //cti分配的设备ID
	TimeStamp_t m_timeStamp; 		//在座席登录后系统分配的时间戳
	
	int m_signIn_sock;                       //初始为-1
	int m_initial_sock;		    	 //initial_socket句柄
	int	m_lMsgReceived;				//描述接收到的消息是否成功
	//用于signIn的socket相关
	std::string m_signIn_IP;
	int m_signIn_Port;

	int m_total_call_num;     
	int m_success_call_num;
	int m_failure_call_num;
	DetailState_t m_curState;
	DetailState_t m_preState;
	
	
	int m_statusAfterCall;		//电话接通后所处的状态：0:后续处理 1：自动空闲 2：自动置忙 3：时间间隔后自动空闲 4：时间间隔后自动置忙
	int m_autoStatusInteval;				//后处理后状态自动切换时间间隔，idleStatus为3或者4时有效后处理后状态自动切换时间间隔，idleStatus为3或者4时有效
				
	CAgent();
	~CAgent();

	std::queue <std::string> m_signIn_msgToSend;   //待发送消息
	std::queue <std::string> m_initial_msgToSend;
};

class CCenter
{
public:
	
	std::map <int, std::queue<std::string> > webSocket;
	
	std::map<int, std::string> socket_agentID_map;  //socket和agent的映射map
	std::map<std::string, CAgent*> agentID_agent_map;
	std::queue<int> socket_Not_In_Epoll;	
	
	int agentNum;
	//std::vector <std::string> agentID;
	int m_totalCall;
	int m_successCall;
	int m_failCall;
	CCenter();
	
};
#endif