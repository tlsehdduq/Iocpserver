#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include<unordered_map>
#include<concurrent_unordered_set.h>
#include <concurrent_priority_queue.h>
#include <sqlext.h>  
#include <string>
#include <random>
#include <set>
#include<queue>
#include<memory>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

enum class EVENT_TYPE : char { EV_INIT, EV_ATTACK, EV_NPC_MOVE };

struct TimerEvent {
	std::chrono::system_clock::time_point wakeupTime;
	int n_id; // monster id 
	int c_id; // client id  
	EVENT_TYPE evtype;
	constexpr bool operator < (const TimerEvent& L) const {
		return (wakeupTime > L.wakeupTime);
	}
	TimerEvent() {}

	TimerEvent(std::chrono::system_clock::time_point w_t, int id, EVENT_TYPE e_id) : wakeupTime(w_t), c_id(id), evtype(e_id)
	{
	}

	TimerEvent(std::chrono::system_clock::time_point w_t, int nid, int cid, EVENT_TYPE e_id) : wakeupTime(w_t), n_id(nid), c_id(cid), evtype(e_id)
	{
	}
};