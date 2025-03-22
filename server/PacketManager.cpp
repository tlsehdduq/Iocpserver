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

	int sectionType = instance.AddToSection(client);
	client->_section = sectionType;

	for (auto& cl : instance._sections[sectionType]._clients) {
		if (cl == client || !cl->_isalive || !instance.CanSee(client, cl)) continue;
		sendAddPacket(client, cl);
		sendAddPacket(cl, client);
	}

	for (auto& npc : instance._sections[sectionType]._npcs) {
		if (instance.CanSee(npc, client)) {
			instance.NpcOn(npc, client);
			sendNpcAddPacket(client, npc);
		}
	}

	vector<int> nearsection = instance.findnearsection(sectionType);
	unordered_set<Session*> nearsectionNpc;
	unordered_set<Session*> nearsectionClient;

	for (int section : nearsection)
	{
		lock_guard<mutex> sectionlock{ client->_lock };
		nearsectionNpc.insert(instance._sections[section]._npcs.begin(), instance._sections[section]._npcs.end());
		nearsectionClient.insert(instance._sections[section]._clients.begin(), instance._sections[section]._clients.end());
	}
	for (auto cl : nearsectionClient)
	{
		if (cl->_isalive == false || instance.CanSee(client, cl) == false)continue;
		sendAddPacket(client, cl);
		sendAddPacket(cl, client);
	}
	for (auto npc : nearsectionNpc)
	{
		if (instance.CanSee(npc, client))
		{
			instance.NpcOn(npc, client);
			sendNpcAddPacket(client, npc);
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
	unordered_set<Session*> sectionnpc;
	{
		std::lock_guard<std::mutex> lock(client->_viewlock);
		curViewList = client->_viewlist;
		sectionnpc = instance._sections[client->_section]._npcs;
	}

	for (auto& session : instance._sections[client->_section]._clients) {
		if (session == client || session->_isalive == false) continue;

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

	for (auto& npc : sectionnpc) {
		if (instance.CanSee(client, npc)&& npc->getHp() > 0) {
			instance.NpcOn(npc, client);
			sendNpcAddPacket(client, npc);

		}
	}
	// ----- nearsection check 
	vector<int> nearsection = instance.findnearsection(client->_section);
	unordered_set<Session*> nearsectionNpc;
	unordered_set<Session*> nearsectionClient;
	for (int section : nearsection)
	{
		lock_guard<mutex> sectionlock{ client->_lock };
		nearsectionNpc.insert(instance._sections[section]._npcs.begin(), instance._sections[section]._npcs.end());
		nearsectionClient.insert(instance._sections[section]._clients.begin(), instance._sections[section]._clients.end());
	}
	for (auto session : nearsectionClient)
	{
		if (session == client)continue;
		if (instance.CanSee(client, session))
		{
			if (curViewList.find(session) == curViewList.end())
			{
				sendAddPacket(session, client);
				sendAddPacket(client, session);
			}
			else
			{
				sendMovePlayerPacket(client, session);
				sendMovePlayerPacket(session, client);
			}
		}
	}
	for (auto npc : nearsectionNpc)
	{
		if (instance.CanSee(client, npc) && npc->_isalive == false) {
			instance.NpcOn(npc, client);

			sendNpcAddPacket(client, npc);
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
	auto& sessionmanager = SessionManager::GetInstance();
	auto& npcs = instance._sections[client->_section]._npcs;
	
	for (auto& npc : npcs) {
		if (client->_leftright && client->getPosX() - 1 == npc->getPosX() && client->getPosY() == npc->getPosY()) {
			npc->setHp(0);
			npc->_isalive = false;
			client->_monstercnt++;
			sendNpcRemovePacket(client, npc);
			break;
		}
		else if (!client->_leftright && client->getPosX() + 1 == npc->getPosX() && client->getPosY() == npc->getPosY()) {
			npc->setHp(0);
			npc->_isalive = false;
			client->_monstercnt++;
			sendNpcRemovePacket(client, npc);			
			break;
		}
	}
	if (client->_monstercnt % 30 == 0)
	{
		int clientlevel = client->getLevel();
		client->setLevel(clientlevel + 1);
		for (auto& cl : sessionmanager._clients)
		{
			if (cl._isalive == false)continue;
			sendPlayerLevelPacket(&cl, client);
		}
	}
}

void PacketManager::HandleChatPacket(Session* client, char* packet) {
	CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);
	auto& instance = SessionManager::GetInstance();

	for (auto& pl : instance._clients) {
		if (pl.getId() == -1) continue;
		sendChatPacket(&pl, client, p->message);
	}
}

void PacketManager::HandleLogoutPacket(Session* client) {
	auto& sectionManager = Map::GetInstance();
	sectionManager._sections[client->_section].RemoveClient(client);

	Gdatabase.saveUserInfo(client->getId());
	auto& manager = SessionManager::GetInstance();
	auto& clientSession = manager._clients[client->getId()];

	clientSession._section = -1;
	clientSession._isalive = false;
	cout << "플레이어 정보를 저장합니다. " << endl;
}

void PacketManager::HandlePlayerUseSkillPacket(Session* client, char* packet)
{
	sendPlayerQSkillPacket(client, client, true);
	auto& instance = Map::GetInstance();
	auto& sessionmanager = SessionManager::GetInstance();
	unordered_set<Session*> npcs = instance._sections[client->_section]._npcs; // <- 같은 섹션에 있는 몬스터 
	vector<int> nearsection = instance.findnearsection(client->_section); 
	pair<short, short> clientpos = { client->getPosX(), client->getPosY() };
	std::unordered_set<Session*, SessionPtrHash, SessionPtrEqual> nearsectionNpc;
	std::unordered_set<Session*, SessionPtrHash, SessionPtrEqual> sectionnpc;
	sectionnpc.insert(npcs.begin(), npcs.end());

	for (int section : nearsection)
	{
		lock_guard<mutex> sectionlock{ client->_lock };
		nearsectionNpc.insert(instance._sections[section]._npcs.begin(),
			instance._sections[section]._npcs.end());
	}
	vector<pair<short, short>> nearpos;
	short directions[8][2] = {
		{-1, -1}, {0, -1}, {1, -1},
		{-1, 0},           {1, 0},
		{-1, 1},  {0, 1},  {1, 1}
	};
	for (auto& dir : directions)
	{
		nearpos.push_back({ clientpos.first + dir[0], clientpos.second + dir[1] });
	}
	vector<Session*> mosters_in_range;
	for (const auto& pos : nearpos)
	{
		Session temp(pos.first, pos.second);
		auto it = sectionnpc.find(&temp);
		if (it != sectionnpc.end())
		{
			mosters_in_range.emplace_back(*it);
		}
		auto it2 = nearsectionNpc.find(&temp);
		if (it2 != nearsectionNpc.end())
		{
			mosters_in_range.emplace_back(*it2);
		}
	}

	for (auto& npc : mosters_in_range)
	{
		npc->_isalive = false;
		npc->setHp(0);
		sendNpcRemovePacket(client, npc);
	}

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
	case CS_PLAYER_SKILL:
		HandlePlayerUseSkillPacket(client, packet);
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

void PacketManager::sendPlayerLevelPacket(Session* from, Session* to)
{
	SC_PlAYER_LEVEL_UP_PACKET p;
	p.size = sizeof(SC_PlAYER_LEVEL_UP_PACKET);
	p.type = SC_PLAYER_LEVELUP;
	p.id = to->getId();
	p.level = to->getLevel();

	from->DoSend(&p);
}

void PacketManager::sendPlayerQSkillPacket(Session* from, Session* to,bool onoff)
{
	SC_PLAYER_SKILL_PACKET p;
	p.size = sizeof(SC_PLAYER_SKILL_PACKET);
	p.type = SC_PLAYER_SKILL;
	p.id = to->getId();
	p.onoff = onoff;
	p.x = to->getPosX();
	p.y = to->getPosY();
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

void PacketManager::sendNpcAttackPacket(Session* npc, Session* client, Session* attackclient)
{
	SC_MONSTER_ATTACK_PACKET p;
	p.size = sizeof(SC_MONSTER_ATTACK_PACKET);
	p.type = SC_MONSTER_ATTACK;
	p.id = npc->getId();
	p.target_id = attackclient->getId();
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


