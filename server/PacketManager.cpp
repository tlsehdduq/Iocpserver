#include "pch.h"
#include "PacketManager.h"
#include "Session.h"
#include "Map.h"

void PacketManager::processData(Session* client, char* packet)
{
	// ���⼭ ������ ���� 
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		client->setName(p->name);
		sendLoginPacket(client);
		auto& instance = Map::GetInstance();
		SectionType roopsectiontype = instance.AddToSection(client);
		client->_section = roopsectiontype;
		for (auto& cl : instance._sections[roopsectiontype]._clients)
		{
			if (cl == client)continue;
			if (instance.CanSee(client, cl) == false)continue;

			sendAddPacket(client, cl); // �ִ� �ֵ����� ���� ���� 
			sendAddPacket(cl, client);
		}
		for (auto& npc : instance._sections[roopsectiontype]._npcs)
		{
			if(instance.CanSee(npc,client))
				instance.NpcOn(npc, client);
		}
		break;
	}
	case CS_MOVE_PLAYER:
	{
		CS_MOVE_PLAYER_PACKET* p = reinterpret_cast<CS_MOVE_PLAYER_PACKET*>(packet);
		// Ŭ���̾�Ʈ �̵� ó��
		client->setMovetime(p->move_time);
		client->Move(p->dir);
		// ���� �� �ν��Ͻ� ��������
		auto& instance = Map::GetInstance();
		// ���� Ȯ�� �� ����
		SectionType currentSection = instance.SectionCheck(client);
		// ���� �� ����Ʈ�� ���� ���� �� ����Ʈ �غ�
		unordered_set<int> newViewList;
		unordered_set<int> oldViewList;
		// ���� �� ����Ʈ ��ȣ
		{
			lock_guard<mutex> lock(client->_viewlock);
			oldViewList = client->_viewlist; // ���� �� ����Ʈ ����
		}
		sendMovePlayerPacket(client, client);
		// ���� ������ Ŭ���̾�Ʈ ����
		for (auto& session : instance._sections[currentSection]._clients)
		{
			// ������ ����
			if (session == client)
				continue;

			// �þ� ���� �� Ȯ��
			if (instance.CanSee(client, session))
			{
				newViewList.insert(session->getId()); // �� �� ����Ʈ�� �߰�

				// ���� ���̴� Ŭ���̾�Ʈ
				if (oldViewList.find(session->getId()) == oldViewList.end())
				{
					if (session->_isNpc)continue;
					sendAddPacket(session, client); // Ŭ���̾�Ʈ���� �߰� ��Ŷ ����
					sendAddPacket(client, session); // ���浵 �����ϰ� ó��
				}
				else
				{
					if (session->_isNpc)continue;
					sendMovePlayerPacket(session, client); // ���� �� ����Ʈ�� �ִ� ��� Move ��Ŷ ����
				}
			}
		}
		for (auto& id : oldViewList)
		{
			if (newViewList.find(id) == newViewList.end())
			{
				auto& instance = SessionManager::GetInstance();
				sendRemovePlayerPacket(client, &instance._clients[id]);
				sendRemovePlayerPacket(&instance._clients[id], client);
			}
		}
		{
			lock_guard<mutex> lock(client->_viewlock);
			client->_viewlist = std::move(newViewList);
		}

		break;
	}
	case CS_ATTACK:
	{
		break;
	}
	case CS_MOVE_NPC:
	{
		break;
	}
	case CS_CHAT:
	{
		break;
	}
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
			// processpacket �̰� ��� ó�����ٱ�? 
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

void PacketManager::sendLoginPacket(Session* session)//���⼭ ������ �޾��شٸ�? 
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
	// to ���� Ŭ���̾�Ʈ 

	from->_viewlock.lock();
	from->_viewlist.insert(to->getId());
	from->_viewlock.unlock();

	from->DoSend(&p); //������ Ŭ���̾�Ʈ 
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

	from->_lock.lock();
	from->_viewlist.erase(to->getId());
	from->_lock.unlock();

	if (from->_isNpc)return;

	from->DoSend(&p);
}

void PacketManager::sendNpcUpdatePacket(Session* from, Session* to)
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

void PacketManager::npcUpdate(Session* npc)
{
	switch (npc->_section)
	{
	case SectionType::LEFTUP:
	{
		auto& instance = Map::GetInstance();

		for (auto& client : instance._sections[SectionType::LEFTUP]._clients)
		{
			sendNpcUpdatePacket(client, npc); //���� ���Ǿȿ� updatepacket 
		}
		break;
	}
	case SectionType::LEFTDOWN:
	{
		auto& instance = Map::GetInstance();

		for (auto& client : instance._sections[SectionType::LEFTDOWN]._clients)
		{
			sendNpcUpdatePacket(client, npc); //���� ���Ǿȿ� updatepacket 
		}
		break;
	}
	case SectionType::RIGHTUP:
	{
		auto& instance = Map::GetInstance();

		for (auto& client : instance._sections[SectionType::RIGHTUP]._clients)
		{
			sendNpcUpdatePacket(client, npc); //���� ���Ǿȿ� updatepacket 
		}
		break;
	}
	case SectionType::RIGHTDOWN:
	{
		auto& instance = Map::GetInstance();

		for (auto& client : instance._sections[SectionType::RIGHTDOWN]._clients)
		{
			sendNpcUpdatePacket(client, npc); //���� ���Ǿȿ� updatepacket 
		}
		break;
	}
	case SectionType::NONE:
	{
		break;
	}
	}
}

bool PacketManager::isEmpty(const Session* session)
{
	return session == nullptr;
}


