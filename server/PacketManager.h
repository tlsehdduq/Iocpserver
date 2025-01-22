#pragma once
#include"types.h"
#include"packet.h"


// PacketManager�� �����ؾ� �� �� 
// Send, Recv 
// ��Ŷ ������ -> workerthread���� ���� 
// processdata ��Ŷ ó�� 
class Session;
class Over;
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
	static void processData(Session* client, char* packet);
	static void recycleData(Session* client,Over* over ,int num_bytes); 
	static void HandleLoginPacket(Session* client, char* packet);
	static void HandleMovePlayerPacket(Session* client, char* packet);
	static void HandleAttackPacket(Session* client);
	static void HandleChatPacket(Session* client, char* packet);
	static void HandleLogoutPacket(Session* client);
	static void sendLoginPacket(Session* session);
	static void sendAddPacket(Session* from, Session* to);
	static void sendMovePlayerPacket(Session* from, Session* to);
	static void sendRemovePlayerPacket(Session* from,Session* to);
	static void sendNpcAddPacket(Session* from, Session* to);
	static void sendNpcMovePacket(Session* from, Session* to);
	static void sendNpcAttackPacket(Session* npc, Session* client);
	static void sendNpcRemovePacket(Session* from, Session* to);
	static void sendChatPacket( Session* from, Session* to,char* message);
	static bool isEmpty(const Session * session);
	
public:
	

};

