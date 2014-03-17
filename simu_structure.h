#include <string>
#include <map>
#include <queue>
#include <sys/time.h>
#include <errno.h>
using namespace std;

typedef char VccID_t[32];
typedef char AgentID_t[32];
typedef char AgentPasswd_t[32];
typedef char SessionID_t[64];
typedef char TimeStamp_t[64];

typedef enum PhoneState_t 
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

typedef enum AgentState_t
{
	AG_NULL = 0,
	AG_NOT_READY = 1,
	AG_READY = 2,
	AG_BUSY = 3,
	AG_WORKING_AFTER_CALL = 4,

}AgentState_t;
typedef enum DetailState_t
{
	AG_NULL = 0,
	AG_TRY_INITIAL,
	AG_INITIAL,
	AG_TRY_SIGNIN,
	AG_SIGNIN,
	AG_TRY_SETIDEL,
	AG_IDLE,
	AG_TRY_SETBUSY,
	AG_BUSY,
	AG_CALLING,
	AG_WORKING_AFTER_CALL,

}

typedef struct Device_t 
{
	PhoneState_t    phonestatus;
    long            legID;
    DeviceID_t      deviceID;
} Device_t;

typedef enum EventClass_t
{
    EC_REQUEST          = 0,
    EC_UNSOLICITED      = 1,
    EC_CONFIRMATION     = 2
} EventClass_t;

typedef enum EventType_t
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


typde struct MsgParser_t
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
	string webIP;
	string webPort;			
	string ctiIP;		
	int ctiPort;
	int agentNum;
	string agentID;
	string logFile;
	string vccID;
	
	CConf();
	int validate();
private:
	DeviceID_t deviceID;

};


class CCenter
{
public:
	int initial_sock;
	int initial_sock_state; //0:disconnected    1:connected
	map < int, queue <string> > webSocket;
	map <string,CAgent*> agentID_CAgent_Map;
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





