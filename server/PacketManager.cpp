#include "pch.h"
#include "PacketManager.h"
#include "Session.h"
#include "Map.h"
#include "DB.h"

DB Gdatabase;

void PacketManager::HandleLoginPacket(Session* client, char* packet) {

	auto& instance = Map::GetInstance();
	CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

	if (!Gdatabase.isAllowAccess(p->name, client->getId()))
		client->setName(p->name);

	sendLoginPacket(client);
	client->_isalive = true;

	SectionType sectionType = instance.AddToSection(client);
	client->_section = sectionType;

	for (auto& cl : instance._sections[sectionType]._clients) {
		if (cl == client || !cl->_isalive || !instance.CanSee(client, cl)) continue;
		sendAddPacket(client, cl);
		sendAddPacket(cl, client);
	}

	for (auto& npc : instance._sections[sectionType]._npcs) {
		if (instance.CanSee(npc, client)) {
			instance.NpcOn(npc, client);
		}
	}
}

void PacketManager::HandleMovePlayerPacket(Session* client, char* packet) {
	auto& instance = Map::GetInstance();
	CS_MOVE_PLAYER_PACKET* p = reinterpret_cast<CS_MOVE_PLAYER_PACKET*>(packet);

	client->setMovetime(p->move_time);
	client->Move(p->dir);

	if (instance.IsNearSectionBoundary(client)) {
		instance.SectionCheck(client);
	}

	sendMovePlayerPacket(client, client);

	unordered_set<Session*> curViewList;
	{
		std::lock_guard<std::mutex> lock(client->_viewlock);
		curViewList = client->_viewlist;
	}

	for (auto& session : instance._sections[client->_section]._clients) {
		if (session == client || !session->_isalive) continue;

		if (instance.CanSee(client, session)) {
			if (curViewList.find(session) == curViewList.end()) {
				sendAddPacket(session, client);
				sendAddPacket(client, session);
			}
			else {
				sendMovePlayerPacket(client, session);
				sendMovePlayerPacket(session, client);
			}
		}
	}

	for (auto& npc : instance._sections[client->_section]._npcs) {
		if (instance.CanSee(client, npc) && !npc->_isalive) {
			instance.NpcOn(npc, client);
		}
	}

	for (auto& session : curViewList) {
		if (!instance.CanSee(session, client)) {
			sendRemovePlayerPacket(client, session);
			sendRemovePlayerPacket(session, client);
		}
	}
}

void PacketManager::HandleAttackPacket(Session* client) {
	auto& instance = Map::GetInstance();
	auto& npcs = instance._sections[client->_section]._npcs;

	for (auto& npc : npcs) {
		if (client->_leftright && client->getPosX() - 1 == npc->getPosX() && client->getPosY() == npc->getPosY()) {
			npc->setHp(0);
			npc->_isalive = false;
			sendNpcRemovePacket(client, npc);
			break;
		}
		else if (!client->_leftright && client->getPosX() + 1 == npc->getPosX() && client->getPosY() == npc->getPosY()) {
			npc->setHp(0);
			npc->_isalive = false;
			sendNpcRemovePacket(client, npc);
			break;
		}
	}
}

void PacketManager::HandleChatPacket(Session* client, char* packet) {
	CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);
	auto& instance = SessionManager::GetInstance();

	for (auto& pl : instance._clients) {
		if (pl.getId() == -1) break;
		sendChatPacket(client, &pl, p->message);
	}
}

void PacketManager::HandleLogoutPacket(Session* client) {
	auto& sectionManager = Map::GetInstance();
	sectionManager._sections[client->_section].RemoveClient(client);

	Gdatabase.saveUserInfo(client->getId());
	auto& manager = SessionManager::GetInstance();
	auto& clientSession = manager._clients[client->getId()];

	clientSession._section = SectionType::NONE;
	clientSession._isalive = false;
}

void PacketManager::processData(Session* client, char* packet) {
	switch (packet[1]) {
	case CS_LOGIN:
		HandleLoginPacket(client, packet);
		break;
	case CS_MOVE_PLAYER:
		HandleMovePlayerPacket(client, packet);
		break;
	case CS_ATTACK:
		HandleAttackPacket(client);
		break;
	case CS_CHAT:
		HandleChatPacket(client, packet);
		break;
	case CS_LOGOUT:
		HandleLogoutPacket(client);
		break;
	default:
		
		break;
	}
}

void PacketManager::recycleData(Session* client, Over* over, int num_bytes)
{

	int remaindata = num_bytes + client->_prevremain;
	char* p = over->_buf;
	while (remaindata > 0)
	{
		int packetsize = p[0];
		if (packetsize <= remaindata)
		{
			processData(client, p);
			p = p + packetsize;
			remaindata = remaindata - packetsize;
		}
		else break;
	}
	client->_prevremain = remaindata;
	if (remaindata > 0)
		memcpy(over->_buf, p, remaindata);

	client->DoRecv();
}

void PacketManager::sendLoginPacket(Session* session)
{
	SC_LOGIN_PACKET p;
	p.size = sizeof(SC_LOGIN_PACKET);
	p.type = SC_LOGIN;
	p.x = session->getPosX();
	p.y = session->getPosY();
	p.id = session->getId();
	p.exp = session->getExp();
	p.hp = session->getHp();

	session->DoSend(&p);
}

void PacketManager::sendAddPacket(Session* from, Session* to)
{
	//to -> from
	SC_ADD_PACKET p;
	p.size = sizeof(SC_ADD_PACKET);
	p.type = SC_ADD_OBJECT;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();
	memcpy(p.name, to->getName(), sizeof(p.name));
	// to 보낼 클라이언트 
	{
		lock_guard<mutex> vl{ from->_viewlock };
		from->_viewlist.insert(to);
	}

	from->DoSend(&p); //보내는 클라이언트 
}

void PacketManager::sendMovePlayerPacket(Session* from, Session* to)
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();
	p.left = to->_leftright;
	p.move_time = to->getMovetime();
	from->DoSend(&p);
}

void PacketManager::sendRemovePlayerPacket(Session* from, Session* to)
{
	SC_REMOVE_PACKET p;
	p.size = sizeof(SC_REMOVE_PACKET);
	p.type = SC_REMOVE;
	p.id = to->getId();
	if (to->_isNpc)
		p.sessiontype = 0;
	else p.sessiontype = 1;

	{
		lock_guard<mutex> vl{ from->_viewlock };
		from->_viewlist.erase(to);
	}

	from->DoSend(&p);
}

void PacketManager::sendNpcAddPacket(Session* from, Session* to)
{
	SC_MONSTER_INIT_PACKET p;
	p.size = sizeof(SC_MONSTER_INIT_PACKET);
	p.type = SC_MONSTER_INIT;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();
	p.max_hp = 100;
	p.att = 5;

	from->DoSend(&p);
}

void PacketManager::sendNpcMovePacket(Session* from, Session* to)
{
	SC_MONSTER_MOVE_PACKET p;
	p.size = sizeof(SC_MONSTER_MOVE_PACKET);
	p.type = SC_MONSTER_MOVE;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();

	from->DoSend(&p);
}

void PacketManager::sendNpcAttackPacket(Session* npc, Session* client)
{
	SC_MONSTER_ATTACK_PACKET p;
	p.size = sizeof(SC_MONSTER_ATTACK_PACKET);
	p.type = SC_MONSTER_ATTACK;
	p.id = npc->getId();

	client->DoSend(&p);
}

void PacketManager::sendNpcRemovePacket(Session* from, Session* to)
{
	SC_REMOVE_PACKET p;
	p.size = sizeof(SC_REMOVE_PACKET);
	p.type = SC_REMOVE;
	p.sessiontype = 0;
	p.id = to->getId();
	p.monstercnt = from->_monstercnt;
	from->DoSend(&p);

}

void PacketManager::sendChatPacket(Session* from, Session* to, char* message)
{
	SC_CHAT_PACKET p;
	p.size = sizeof(SC_CHAT_PACKET);
	p.type = SC_CHAT;
	p.id = to->getId();
	memcpy(p.message, message, sizeof(message));
	from->DoSend(&p);
}

bool PacketManager::isEmpty(const Session* session)
{
	return session == nullptr;
}


