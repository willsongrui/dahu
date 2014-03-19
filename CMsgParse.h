#include "simudef.h"
#include <string>
#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"
using namespace std;
using namespace rapidxml;
class CMsgParse  
{
public:
	vector<string>   m_arrCmd;
	vector<int>      m_arrCmdIndex;
	CMsgParse();
	CMsgParse(string str);
	virtual ~CAcpParse();

    BOOL ParseMsg(ACPEvent_t &msg,string str);
    void BuildIntialConf(ACPInitialConfEvent_t &initialConf,TXMLHandle hBody);
    void BuildGeneralConf(ACPGeneralConfEvent_t &generalConf,TXMLHandle hBody);
    void BuildGeneralEventReport(ACPEventReportEvent_t &generalEventReport,TXMLHandle hBody);
    void BuildHangupCallInfo(ACPHangupCallEvent_t &hangupCallEventReport,TXMLHandle hBody);
	void BuildRecordInfo(ACPRecordInfoEvent_t &recordInfoEventReport,TXMLHandle hBody);
    void BuildCallinfo(Callinfo_t &callInfo,TXMLHandle hBody);
    void BuildAgentInfo(Agent_t &agentInfo,TXMLHandle hBody);
    void BuildCauseInfo(Cause_t &causeInfo,TXMLHandle hBody);
    void BuildCallinfoEventReport(ACPEvent_t &msg,TXMLHandle hBody);
    void BuildAnswerRequestEventReport(ACPEvent_t &msg,TXMLHandle hBody);
    void BuildReleaseEventReport(ACPEvent_t &msg,TXMLHandle hBody);

};