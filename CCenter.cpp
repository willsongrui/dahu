#include "simu_def.h"
using namespace std;
CConf::CConf()
{
	ipc = "";
	ctiIp = "";
	ctiPort = 0;
	agentNum = 0;
	agentID = 0;
	logFile = 0;


	eventTypeMap.insert(make_pair("heartBeat", ACP_HEART_BEAT));	
	eventTypeMap.insert(make_pair("Initial", ACP_Initial_CONF));	
	eventTypeMap.insert(make_pair("SignIn", ACP_SignIn_CONF));	
	eventTypeMap.insert(make_pair("SignOut", ACP_SignOut_CONF));
	eventTypeMap.insert(make_pair("SetBusy", ACP_SetBusy_CONF));	
	eventTypeMap.insert(make_pair("SetIdle", ACP_SetIdle_CONF));	
	eventTypeMap.insert(make_pair("AgentReport", ACP_AgentReport_CONF));	
	eventTypeMap.insert(make_pair("SetAgentAutoEnterIdle", ACP_SetAgentAutoEnterIdle_CONF));
	eventTypeMap.insert(make_pair("AgentEnterIdle", ACP_AgentEnterIdle_CONF));	
	eventTypeMap.insert(make_pair("AgentEnterWrapUp", ACP_AgentEnterWrapUp_CONF));	
	eventTypeMap.insert(make_pair("ForceIdle", ACP_ForceIdle_CONF));
	eventTypeMap.insert(make_pair("ForceBusy", ACP_ForceBusy_CONF));	
	eventTypeMap.insert(make_pair("ForceOut", ACP_ForceOut_CONF));	
	eventTypeMap.insert(make_pair("ReleaseCall", ACP_ReleaseCall_CONF));	
	eventTypeMap.insert(make_pair("ReleaseCallByCallID", ACP_ReleaseCallByCallID_CONF));	
	eventTypeMap.insert(make_pair("SendDTMF", ACP_SendDTMF_CONF));
	eventTypeMap.insert(make_pair("CallInside", ACP_CallInside_CONF));
	eventTypeMap.insert(make_pair("Consultation", ACP_Consultation_CONF));
	eventTypeMap.insert(make_pair("CallOutside", ACP_CallOutside_CONF));
	eventTypeMap.insert(make_pair("SingleStepTransfer", ACP_SingleStepTransfer_CONF));

	eventTypeMap.insert(make_pair("Transfer", ACP_Transfer_CONF));
	eventTypeMap.insert(make_pair("SetTransfer", ACP_SetTransfer_CONF));
	eventTypeMap.insert(make_pair("Hold", ACP_Hold_CONF));
	eventTypeMap.insert(make_pair("RetrieveHold", ACP_RetrieveHold_CONF));
	eventTypeMap.insert(make_pair("Conference", ACP_Conference_CONF));
	eventTypeMap.insert(make_pair("Alternate", ACP_Alternate_CONF));
	eventTypeMap.insert(make_pair("DoReplyMultimeAndHangUpCall", ACP_DoReplyMultimeAndHangUpCall_CONF));
	eventTypeMap.insert(make_pair("Listen", ACP_Listen_CONF));
	eventTypeMap.insert(make_pair("Intercept", ACP_Intercept_CONF));
	eventTypeMap.insert(make_pair("Insert", ACP_Insert_CONF));
	eventTypeMap.insert(make_pair("StopInsert", ACP_StopInsert_CONF));
	eventTypeMap.insert(make_pair("BeginRecord", ACP_BeginRecord_CONF));
	eventTypeMap.insert(make_pair("StopRecord", ACP_StopRecord_CONF));
	eventTypeMap.insert(make_pair("Bridge", ACP_Bridge_CONF));
	eventTypeMap.insert(make_pair("SetCallData", ACP_SetCallData_CONF));
	eventTypeMap.insert(make_pair("GetCallData", ACP_GetCallData_CONF));
	eventTypeMap.insert(make_pair("QueryTotalQueueWaitNum", ACP_QueryTotalQueueWaitNum_CONF));
	eventTypeMap.insert(make_pair("QueryQueueWaitNum", ACP_QueryQueueWaitNum_CONF));
	eventTypeMap.insert(make_pair("QueryAgentStatus", ACP_QueryAgentStatus_CONF));
	eventTypeMap.insert(make_pair("GetAccessNo", ACP_GetAccessNo_CONF));
	eventTypeMap.insert(make_pair("QueryAgentCallInfo", ACP_QueryAgentCallInfo_CONF));
	eventTypeMap.insert(make_pair("SetCTIParam", ACP_SetCTIParam_CONF));

	
	eventTypeMap.insert(make_pair("OnOrigated", ACP_OnOrigated));
	eventTypeMap.insert(make_pair("OnAnswerRequest", ACP_OnAnswerRequest));
	eventTypeMap.insert(make_pair("OnAnswerSuccess", ACP_OnAnswerSuccess));
	eventTypeMap.insert(make_pair("OnAnswerFailure", ACP_OnAnswerFailure));
	eventTypeMap.insert(make_pair("OnUserHangupBeforeResponse", ACP_OnUserHangupBeforeResponse));
	eventTypeMap.insert(make_pair("OnRequestRelease", ACP_OnRequestRelease));	
	eventTypeMap.insert(make_pair("OnReleaseSuccess", ACP_OnReleaseSuccess));
	eventTypeMap.insert(make_pair("OnReleaseFailure", ACP_OnReleaseFailure));
	eventTypeMap.insert(make_pair("OnLongNoAnswer", ACP_OnLongNoAnswer));
	eventTypeMap.insert(make_pair("OnConsultationBack", ACP_OnConsultationBack));
	eventTypeMap.insert(make_pair("OnConsultationSuccess", ACP_OnConsultationSuccess));
	eventTypeMap.insert(make_pair("OnConsultationFailure", ACP_OnConsultationFailure));
	eventTypeMap.insert(make_pair("OnTransSuccess", ACP_OnTransSuccess));
	eventTypeMap.insert(make_pair("OnTransFailure", ACP_OnTransFailure));
	eventTypeMap.insert(make_pair("OnReturnFromIVR", ACP_OnReturnFromIVR));
	eventTypeMap.insert(make_pair("OnReturnFromPhone", ACP_OnReturnFromPhone));
	eventTypeMap.insert(make_pair("OnBridgeSuccess", ACP_OnBridgeSuccess));
	eventTypeMap.insert(make_pair("OnBridgeFailure", ACP_OnBridgeFailure));

	
	eventTypeMap.insert(make_pair("OnSingleTransferSuccess", ACP_OnSingleTransferSuccess));
	eventTypeMap.insert(make_pair("OnSingleTransferFailure", ACP_OnSingleTransferFailure));
	eventTypeMap.insert(make_pair("OnCallInsideSuccess", ACP_OnCallInsideSuccess));
	eventTypeMap.insert(make_pair("OnCallInsideFailure", ACP_OnCallInsideFailure));
	eventTypeMap.insert(make_pair("OnCallInsideSucTalk", ACP_OnCallInsideSucTalk));
	eventTypeMap.insert(make_pair("OnCallOutsideSuccess", ACP_OnCallOutsideSuccess));
	eventTypeMap.insert(make_pair("OnCallOutsideFailure", ACP_OnCallOutsideFailure));
	eventTypeMap.insert(make_pair("OnCallOutsideSuccTalk", ACP_OnCallOutsideSuccTalk));
	eventTypeMap.insert(make_pair("OnHoldSuccess", ACP_OnHoldSuccess));
	eventTypeMap.insert(make_pair("OnHoldFailure", ACP_OnHoldFailure));
	eventTypeMap.insert(make_pair("OnRetrieveHoldSuccess" ACP_OnRetrieveHoldSuccess));
	eventTypeMap.insert(make_pair("OnRetrieveHoldFailure", ACP_OnRetrieveHoldFailure));
	eventTypeMap.insert(make_pair("OnHoldCallRelease", ACP_OnHoldCallRelease));
	eventTypeMap.insert(make_pair("OnConferenceSuccess", ACP_OnConferenceSuccess));
	eventTypeMap.insert(make_pair("OnConferenceFailure", ACP_OnConferenceFailure));
	eventTypeMap.insert(make_pair("OnConferenceSucTalk", ACP_OnConferenceSucTalk));
	eventTypeMap.insert(make_pair("OnHangupCallInConf", ACP_OnHangupCallInConf));
	eventTypeMap.insert(make_pair("OnAlternateSuccess", ACP_OnAlternateSuccess));
	eventTypeMap.insert(make_pair("OnAlternated", ACP_OnAlternated));
	eventTypeMap.insert(make_pair("OnAlternateFailure", ACP_OnAlternateFailure));
	
	eventTypeMap.insert(make_pair("OnListenSuccess", ACP_OnListenSuccess));
	eventTypeMap.insert(make_pair("OnListenFailure", ACP_OnListenFailure));
	eventTypeMap.insert(make_pair("OnInsertSuccess", ACP_OnInsertSuccess));
	eventTypeMap.insert(make_pair("OnInsertFailure", ACP_OnInsertFailure));
	eventTypeMap.insert(make_pair("OnStopInsertSuccess", ACP_OnStopInsertSuccess));
	eventTypeMap.insert(make_pair("OnStopInsertFailure", ACP_OnStopInsertFailure));
	eventTypeMap.insert(make_pair("OnInterceptSuccess", ACP_OnInterceptSuccess));
	eventTypeMap.insert(make_pair("OnInterceptFailure", ACP_OnInterceptFailure));
	eventTypeMap.insert(make_pair("OnForceIdleSuccess", ACP_OnForceIdleSuccess));
	eventTypeMap.insert(make_pair("OnForceIdleFailure", ACP_OnForceIdleFailure));
	
	
	eventTypeMap.insert(make_pair("OnForceBusySuccess", ACP_OnForceBusySuccess));
	eventTypeMap.insert(make_pair("OnForceBusyFailure", ACP_OnForceBusyFailure));
	eventTypeMap.insert(make_pair("OnForceOutSuccess", ACP_OnForceOutSuccess));
	eventTypeMap.insert(make_pair("OnForceOutFailure", ACP_OnForceOutFailure));
	eventTypeMap.insert(make_pair("OnBeginRecordSuccess", ACP_OnBeginRecordSuccess));
	eventTypeMap.insert(make_pair("OnBeginRecordFailure", ACP_OnBeginRecordFailure));
	eventTypeMap.insert(make_pair("OnStopRecordSuccess", ACP_OnStopRecordSuccess));
	eventTypeMap.insert(make_pair("OnStopRecordFailure", ACP_OnStopRecordFailure));
	eventTypeMap.insert(make_pair("OnForceIdle", ACP_OnForceIdle));
	eventTypeMap.insert(make_pair("OnForceBusy", ACP_OnForceBusy));
	eventTypeMap.insert(make_pair("OnForceOut", ACP_OnForceOut));
	eventTypeMap.insert(make_pair("OnSignOuted", ACP_OnSignOuted));
	eventTypeMap.insert(make_pair("OnForceRelease", ACP_OnForceRelease));
	eventTypeMap.insert(make_pair("OnAgentAllBusy", ACP_OnAgentAllBusy));
	eventTypeMap.insert(make_pair("OnIntercepted", ACP_OnIntercepted));
	eventTypeMap.insert(make_pair("OnInserted", ACP_OnInserted));
	eventTypeMap.insert(make_pair("OnStopListenSuccess", ACP_OnStopListenSuccess));
	eventTypeMap.insert(make_pair("OnStopListenFailure", ACP_OnStopListenFailure));
	eventTypeMap.insert(make_pair("OnListened", ACP_OnListened));
	eventTypeMap.insert(make_pair("OnSystemIdle", ACP_OnListened));
	eventTypeMap.insert(make_pair("OnQueueReport", ACP_OnQueueReport));
	eventTypeMap.insert(make_pair("OnSystemMessage", ACP_OnSystemMessage));

}
