#include "simu_def.h"
using namespace std;



CCenter::CCenter()
{
	while(webSocket.empty()==false)
		webSocket.pop();
	socket_agentID_map.clear();
	agentID_agent_map.clear();
	while(socket_Not_In_Epoll.empty()==false)
		socket_Not_In_Epoll.pop();
	agentID.clear();
	totalCall = successCall = failCall = 0;

}