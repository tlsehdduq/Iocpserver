#pragma once
#include"types.h"
#include"packet.h"
#include"Session.h"

// PacketManager가 수행해야 할 일 
// Send, Recv 
// 패킷 재조립 -> workerthread에서 수행 
// processdata 패킷 처리 
enum class PacketType : char
{
	LOGIN,
	ADD,
	MOVE,
	REMOVE,
	ATTACK
};
class PacketManager
{
public:
	static void processData(SessionRef client, char* packet);
	static void recycleData(SessionRef client, shared_ptr<Over> over ,int num_bytes); 
	static void sendLoginPacket(SessionRef session);
	static void sendAddPacket(SessionRef from, SessionRef to);
	static void sendMovePlayerPacket(SessionRef from, SessionRef to);
	static void sendRemovePlayerPacket(SessionRef session);


};

