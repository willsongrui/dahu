int parseMsg(ACPEvent_t &msg,string str,CLog* log)
{
	if(log == NULL)
		return -1;
	int pos;
	pos = str.find("<acpMessage");
	if(pos == str.end())
	{
		return -1;
	}
	str = str.substr(pos);
	xml_document<> doc;
	try
	{
		doc.parse<0>(str.c_str());
	}
	catch (parse_error e)
	{
		log->ERROR("Parse error: %s",e.what());
		return -1;
	}

	xml_node<>* acpMessage = doc.first_node("acpMessage");
	if(acpMessage == 0)
	{
		log->ERROR("Get node acpMessage error");
		return -1;
	}
	xml_node<>* header = acpMessage->first_node("header");
	if(header == 0)
	{
		log->ERROR("Get node header error");
		delete acpMessage;
		return -1;
	}
	xml_node<>* sessionID = header->first_node("sessionID");
	if(sessionID!=0)
	{
		size_t len = sizeof(sessionID->value());
		if(len!=0)
			strcpy(msg.eventHeader.sessionID, sessionID->value(), len);
	}

	xml_node<>* timeStamp = header->first_node("timeStamp");
	if(timeStamp!=0)
	{
		size_t len = sizeof(timeStamp->value());
		strcpy(msg.eventHeader.timeStamp, timeStamp->value(), len);
	}

	xml_node<>* body = acpMessage->first_node("body");
	if(body == 0)
	{
		log->ERROR("Get node body error");
		delete acpMessage;
		delete header;
		return -1;	
	}
	xml_attribute<>* type = body->first_attribute("type");
	xml_attribute<>* name = body->first_attribute("name");
	if((type == 0)||(name == 0))
	{
		log->ERROR("Get attribute type or name error");
		delete acpMessage;
		delete header;
		delete body;
		return -1;
	}

	if(strcmp(type->value(),"request") == 0)
	{
		msg.eventHeader.eventClass = EC_REQUEST;
	}
	else if(strcmp(type->value(),"response") == 0)
	{
		msg.eventHeader.eventClass = EC_CONFIRMATION;
	}
	else if(strcmp(type->value(),"event") == 0)
	{
		msg.eventHeader.eventClass = EC_UNSOLICITED;
	} 
	else
	{
		log->ERROR("不认识的type: %s",type->value());
		delete acpMessage;
		delete header;
		delete body;
		delete type;
		delete name;
		return -1;
	}


	str = str.Right(str.GetLength()-str.Find("<"));

	BOOL bReturn = TRUE;
	TXMLHandle hRoot = xmlGetTree(str);
	if (hRoot != 0) 
	{
		if (xmlGetValue(hRoot, "acpMessage/header/sessionID",
			msg.eventHeader.sessionID, sizeof(SessionID_t)) >-1)
		{
		
			//timeStamp
			strcpy(msg.eventHeader.timeStamp,"");
			if(xmlGetElememt(hRoot, "acpMessage/header/timeStamp"))
				xmlGetValue(hRoot, "acpMessage/header/timeStamp",msg.eventHeader.timeStamp, sizeof(SessionID_t));

			// save sessionID
			TXMLHandle hBody = xmlGetElememt(hRoot, "acpMessage/body");
			if (hBody != 0)
			{
				const char* clas = sXmlGetAttrValue(hBody, "type");
				const char* type = sXmlGetAttrValue(hBody, "name");
				
				// set event class
				if (strcmp(clas, "request") == 0) 
				{
					msg.eventHeader.eventClass = EC_REQUEST;
				} 
				else if (strcmp(clas, "response") == 0)
				{
					msg.eventHeader.eventClass = EC_CONFIRMATION;
				}
				else if (strcmp(clas, "event") == 0) 
				{
					msg.eventHeader.eventClass = EC_UNSOLICITED;
				}
				else 
				{
					ASSERT(FALSE);
					bReturn = FALSE;
					 xmlFreeTree(hRoot);
					return bReturn;
				}
				
				// set event type
				msg.eventHeader.eventType = ACP_UNKNOWN;
				int index = 0;
				for (const char* p=m_arrCmd[0]; p!=NULL; ++index, p=m_arrCmd[index])
				{
					if (strcmp(p, type) == 0)
					{
						msg.eventHeader.eventType = (EventType_t) m_arrCmdIndex.GetAt(index);
						break;
					}
				}
				// set event body
				switch (msg.eventHeader.eventType) 
				{
				case ACP_Initial_CONF:
					BuildIntialConf(msg.event.acpConfirmation.u.initialConf,hBody);
					break;
				case ACP_SignIn_CONF:
					{
						BuildGeneralConf(msg.event.acpConfirmation.u.signOutcConf.parameter,hBody);
						TXMLHandle hAgentParam   = xmlGetElememt(hBody, "agentParam");
						if(hAgentParam)
						{
							msg.event.acpConfirmation.u.signOutcConf.agentParam.idleStatus = 0;
							const char* temp = sXmlGetAttrValue(hAgentParam,"idleStatus");
							if(temp)
								msg.event.acpConfirmation.u.signOutcConf.agentParam.idleStatus = atoi(temp);

							strcpy(msg.event.acpConfirmation.u.signOutcConf.agentParam.groupID,sXmlGetAttrValue(hAgentParam,"locked"));

							msg.event.acpConfirmation.u.signOutcConf.agentParam.agentType = 0;
							temp = sXmlGetAttrValue(hAgentParam,"agentType");
							if(temp)
								msg.event.acpConfirmation.u.signOutcConf.agentParam.agentType = atoi(temp);

							msg.event.acpConfirmation.u.signOutcConf.agentParam.locked = 0;
							temp = sXmlGetAttrValue(hAgentParam,"locked");
							if(temp)
								msg.event.acpConfirmation.u.signOutcConf.agentParam.locked = atoi(temp);

							msg.event.acpConfirmation.u.signOutcConf.agentParam.allTimeRecord = 0;
							temp = sXmlGetAttrValue(hAgentParam,"allTimeRecord");
							if(temp)
								msg.event.acpConfirmation.u.signOutcConf.agentParam.allTimeRecord = atoi(temp);

							msg.event.acpConfirmation.u.signOutcConf.agentParam.deviceType = 0;//ÆÕÍ¨
							temp = sXmlGetAttrValue(hAgentParam,"deviceType");
							if(temp)
								msg.event.acpConfirmation.u.signOutcConf.agentParam.deviceType = atoi(temp);
							
							temp = sXmlGetAttrValue(hAgentParam,"ctiEvent");
							
							if(temp)
								strcpy(msg.event.acpConfirmation.u.signOutcConf.agentParam.ctiEvent,temp);
							else
								strcpy(msg.event.acpConfirmation.u.signOutcConf.agentParam.ctiEvent,"");
						}
					}
					break;
				case ACP_SignOut_CONF:
				case ACP_SetBusy_CONF:
				case ACP_SetIdle_CONF:
				case ACP_AgentReport_CONF:
				case ACP_SetAgentAutoEnterIdle_CONF:
				case ACP_AgentEnterIdle_CONF:
				case ACP_AgentEnterWrapUp_CONF:
				case ACP_ForceIdle_CONF:
				case ACP_ForceBusy_CONF:
				case ACP_ForceOut_CONF:
				case ACP_ReleaseCall_CONF:
				case ACP_ReleaseCallByCallID_CONF:
				case ACP_SendDTMF_CONF:
				case ACP_CallInside_CONF:
				case ACP_Consultation_CONF:
				case ACP_CallOutside_CONF:
				case ACP_SingleStepTransfer_CONF:
				case ACP_Transfer_CONF:
				case ACP_SetTransfer_CONF:
				case ACP_Hold_CONF:
				case ACP_RetrieveHold_CONF:
				case ACP_Conference_CONF:
				case ACP_Alternate_CONF:
				case ACP_DoReplyMultimeAndHangUpCall_CONF:
				case ACP_Listen_CONF:
				case ACP_Intercept_CONF:
				case ACP_Insert_CONF:
				case ACP_StopInsert_CONF:
				case ACP_BeginRecord_CONF:
				case ACP_StopRecord_CONF:
				case ACP_Bridge_CONF:
				case ACP_SetCTIParam_CONF:
				case ACP_SetCallData_CONF:
					BuildGeneralConf(msg.event.acpConfirmation.u.generalConf,hBody);
					break;
				case ACP_QueryAgentStatus_CONF:
					BuildGeneralConf(msg.event.acpConfirmation.u.queryagentstatus.parameter,hBody);
					BuildAgentInfo(msg.event.acpConfirmation.u.queryagentstatus.destAgentInfo,hBody);
					break;
				case ACP_QueryAgentCallInfo_CONF:
					BuildGeneralConf(msg.event.acpConfirmation.u.queryAgentCallInfo.parameter,hBody);
					BuildAgentInfo(msg.event.acpConfirmation.u.queryAgentCallInfo.destAgentInfo,hBody);
					BuildCallinfo(msg.event.acpConfirmation.u.queryAgentCallInfo.callinfo,hBody);
					break;
				//alerting
				case ACP_OnOrigated:
					BuildGeneralEventReport(msg.event.acpEventReport.u.origatedEventReport.parameter,hBody);
					BuildCallinfo(msg.event.acpEventReport.u.origatedEventReport.callInfo,hBody);	
					break;
				case ACP_OnAnswerRequest:
					BuildAnswerRequestEventReport(msg,hBody);
					break;
				//CallingNo="" CalledNo=""
				case ACP_OnHangupCallInConf:
					BuildHangupCallInfo(msg.event.acpEventReport.u.hangupCallEventReport,hBody);
					break;
				case ACP_OnBeginRecordSuccess:
					BuildRecordInfo(msg.event.acpEventReport.u.recordInfoEventReport,hBody);
					break;
				case ACP_OnForceIdle:
				case ACP_OnForceBusy:
				case ACP_OnForceOut:
					break;
				case ACP_GetCallData_CONF:
					{
						BuildGeneralEventReport(msg.event.acpConfirmation.u.getCallData.parameter,hBody);
						
						TXMLHandle hCallData   = xmlGetElememt(hBody, "callData");
						if(hCallData)
						{
							const char* callData = sXmlGetValue(hCallData);
							if(callData)
								strcpy(msg.event.acpConfirmation.u.getCallData.callData,callData);
						}
					}
					break;
				case ACP_OnRequestRelease:
				case ACP_OnHoldCallRelease:
					BuildReleaseEventReport(msg,hBody);
					break;
				//include callinfo
				case ACP_OnReturnFromIVR:
				case ACP_OnConsultationBack:
				case ACP_OnReturnFromPhone:
				case ACP_OnReleaseSuccess:
					BuildReleaseEventReport(msg,hBody);
					BuildCallinfo(msg.event.acpEventReport.u.releaseEventReport.callInfo,hBody);
					break;
				case ACP_OnAnswerSuccess:
				case ACP_OnConsultationSuccess:
				case ACP_OnCallInsideSucTalk:
				case ACP_OnCallOutsideSuccTalk:
				case ACP_OnHoldSuccess:
				case ACP_OnRetrieveHoldSuccess:
				case ACP_OnConferenceSuccess:
				case ACP_OnConferenceSucTalk:
				case ACP_OnAlternateSuccess:
				case ACP_OnAlternated:
				case ACP_OnListenSuccess:
				case ACP_OnInsertSuccess:
				case ACP_OnInterceptSuccess:
				case ACP_OnIntercepted:
				case ACP_OnInserted:
				case ACP_OnSignOuted:
				case ACP_OnBridgeSuccess:
					BuildCallinfoEventReport(msg,hBody);
					break;
				//no callinfo 
				case ACP_OnAnswerFailure:
				case ACP_OnUserHangupBeforeResponse:
				case ACP_OnReleaseFailure:
				case ACP_OnLongNoAnswer:
				case ACP_OnConsultationFailure:
				case ACP_OnTransSuccess:
				case ACP_OnTransFailure:	
 				case ACP_OnSingleTransferSuccess:
				case ACP_OnSingleTransferFailure:
				case ACP_OnCallInsideSuccess:
				case ACP_OnCallInsideFailure:
				case ACP_OnCallOutsideSuccess:
				case ACP_OnCallOutsideFailure:
				case ACP_OnHoldFailure:
				case ACP_OnRetrieveHoldFailure:
				case ACP_OnConferenceFailure:
				case ACP_OnAlternateFailure:
				case ACP_OnListenFailure:
				case ACP_OnInsertFailure:
				case ACP_OnStopInsertSuccess:
				case ACP_OnStopInsertFailure:
				case ACP_OnInterceptFailure:
				case ACP_OnForceIdleSuccess:
				case ACP_OnForceIdleFailure:
				case ACP_OnForceBusySuccess:	
				case ACP_OnForceBusyFailure:	
				case ACP_OnForceOutSuccess:	
				case ACP_OnForceOutFailure:	
				case ACP_OnBeginRecordFailure:
				case ACP_OnStopRecordSuccess:
				case ACP_OnStopRecordFailure:
				case ACP_OnForceRelease:
				case ACP_OnAgentAllBusy:
				case ACP_OnStopListenSuccess:
				case ACP_OnStopListenFailure:
				case ACP_OnListened:
				case ACP_OnBridgeFailure:
				case ACP_OnSystemMessage:
					BuildGeneralEventReport(msg.event.acpEventReport.u.generalEventReport,hBody);
					break;
				case ACP_HEART_BEAT:
					break;					
				default:
                    //ASSERT(FALSE);
					bReturn = FALSE;
                }
            }
            else
            {
                ASSERT(FALSE);
				bReturn = FALSE;
            }
            
            xmlFreeTree(hRoot);
        }
        else 
        {
            TRACE1("bad message format. message: \n %s", str);
            ASSERT(FALSE);
            xmlFreeTree(hRoot);
			bReturn = FALSE;
        }
	}
	return bReturn;
}



