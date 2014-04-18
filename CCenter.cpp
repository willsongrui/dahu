#include "simu_def.h"
using namespace std;



CCenter::CCenter()
{
	webSocket.clear();
	socket_agentID_map.clear();
	agentID_agent_map.clear();
	ready_to_send.clear();
	while(socket_Not_In_Epoll.empty()==false)
		socket_Not_In_Epoll.pop();
	
	m_totalCall = m_successCall = m_failCall = 0;
	detail_state_dict.insert(make_pair(AGENT_FRESH, string("AGENT_FRESH")));
	detail_state_dict.insert(make_pair(AGENT_INITIAL, string("AGENT_INITIAL")));
	detail_state_dict.insert(make_pair(AGENT_IDLE, string("AGENT_IDLE")));
	detail_state_dict.insert(make_pair(AGENT_BUSY, string("AGENT_BUSY")));
	detail_state_dict.insert(make_pair(AGENT_SIGNIN, string("AGENT_SIGNIN")));
	detail_state_dict.insert(make_pair(AGENT_ANSWERING, string("AGENT_ANSWERING")));
	detail_state_dict.insert(make_pair(AGENT_SIGNOUT, string("AGENT_SIGNOUT")));
	detail_state_dict.insert(make_pair(AGENT_RELEASE, string("AGENT_RELEASE")));


}