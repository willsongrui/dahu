#ifndef __simu_def__
#define __simu_def__
#include <string>
#include <map>
#include <queue>
#include <sys/time.h>
#include <errno.h>

typedef char VccID_t[32];
typedef char AgentID_t[32];
typedef char AgentPasswd_t[32];
typedef char SessionID_t[64];
typedef char TimeStamp_t[64];
typedef char DeviceID_t[64];
const int MSG_FAILURE = -1;
const int MSG_SUCCESS = 0;
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
typedef struct Agent_t
{
    int          type; 
    VccID_t      vccID; 
    AgentID_t    agentID;
	DeviceID_t   deviceID;
	AgentState_t agentStatus;
	PhoneState_t phoneStatus;
	int          master;//0|1
} Agent_t;
typedef struct Cause_t  
{
    int             code;
    Description_t   desc;
} Cause_t;
typedef struct Device_t 
{
	PhoneState_t    phonestatus;
    long            legID;
    DeviceID_t      deviceID;
} Device_t;

typedef struct Parameter_t
{
	DeviceID_t callID;
	Agent_t    agent;
	Cause_t    cause;
} Parameter_t;




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


typedef struct ACPEventHeader_t 
{
    SessionID_t    sessionID;
    SessionID_t     serialID;
	int             serviceID;
    TimeStamp_t     timeStamp;
    EventClass_t   eventClass;
    EventType_t    eventType;
} ACPEventHeader_t;

typedef Parameter_t ACPGeneralConfEvent_t;

typedef struct ACPAgentParam_t
{
	int             idleStatus;  //ºô½Ð½áÊøºó´¦Àí±êÖ¾0£º±£³ÖÔÚAgentWorkingAfterCall×´Ì¬1£º×Ô¶¯½øÈëAgentReady×´Ì¬
	Description_t   groupID;    //
    int             agentType ;//0£ºÆÕÍ¨»°ÎñÔ±1£º°à³¤2£ºÖÊ¼ìÔ±
    int             locked; //0£ºÎ´Ëø¶¨1£ºÒÑËø¶¨
	int             allTimeRecord;  //0£º²»Â¼  1£ºÆÕÍ¨Â¼Òô
	int             deviceType;     //0£º²»Â¼  1£ºÆÕÍ¨Â¼Òô
	Description_t   ctiEvent;       //CTI×Ô¶¯»úºÅ
} ACPAgentParam_t;

typedef struct ACPSignOutConfEvent_t
{
    Parameter_t      parameter ;
	ACPAgentParam_t  agentParam ;
} ACPSignOutConfEvent_t;


typedef struct ACPConfirmationEvent_t 
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
} ACPConfirmationEvent_t;

typedef Parameter_t ACPEventReportEvent_t;

typedef struct ACPUnsolicitedEvent_t
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

typedef struct ACPEvent_t
{
    ACPEventHeader_t eventHeader;
    union
    {
        ACPConfirmationEvent_t    acpConfirmation;
        ACPUnsolicitedEvent_t     acpEventReport;
    } event;
} ACPEvent_t;

typedef struct MsgParser_t
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
	
	int webPort;			
	std::string ctiIP;		
	int ctiPort;
	int agentNum;
	std::string agentID;
	std::string logFile;
	std::string vccID;
	int ready;
	std::string passwd;
};

typedef struct serverInfo
{
 //  <N-CTS ip="192.168.2.217" port="24001"/>	CString dn;
	string Ip;
	long    port;
	string protocol;
}serverInfo;
#endif