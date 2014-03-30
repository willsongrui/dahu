#include "simu_def.h"
using namespace std;



CCenter::CCenter()
{
	webSocket.clear();
	socket_agentID_map.clear();
	agentID_agent_map.clear();
	while(socket_Not_In_Epoll.empty()==false)
		socket_Not_In_Epoll.pop();
	
	m_totalCall = m_successCall = m_failCall = 0;

}