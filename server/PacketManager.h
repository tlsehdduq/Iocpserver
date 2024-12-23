#pragma once
#include"types.h"
#include"packet.h"
#include"Session.h"

// PacketManager�� �����ؾ� �� �� 
// Send, Recv 
// ��Ŷ ������ -> workerthread���� ���� 
// processdata ��Ŷ ó�� 
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

