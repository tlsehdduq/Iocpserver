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
			if (instance.CanSee(client,cl) == false)continue;
			sendAddPacket(client, cl); // �ִ� �ֵ����� ���� ���� 
			sendAddPacket(cl, client);
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
					sendAddPacket(session,client); // Ŭ���̾�Ʈ���� �߰� ��Ŷ ����
					sendAddPacket(client,session); // ���浵 �����ϰ� ó��
				}
				else
				{
					sendMovePlayerPacket(session,client); // ���� �� ����Ʈ�� �ִ� ��� Move ��Ŷ ����
				}
			}
		}

		// ���� �� ����Ʈ�� �� �� ����Ʈ ��
		for (auto& id : oldViewList)
		{
			// �� �̻� ������ �ʴ� Ŭ���̾�Ʈ
			if (newViewList.find(id) == newViewList.end())
			{
				sendRemovePlayerPacket(client,)
				client->sendRemovePacket(id, 1); // Ŭ���̾�Ʈ���� ���� ��Ŷ ����
				auto& session = _clients[id];
				session->sendRemovePacket(client->getId(), 1); // ���浵 �����ϰ� ó��
			}
		}

		// �� ����Ʈ ����
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

void PacketManager::sendRemovePlayerPacket(Session* from,Session* to)
{
	SC_REMOVE_PACKET p;
	p.size = sizeof(SC_REMOVE_PACKET);
	p.type = SC_REMOVE;
	p.id = to->getId();

	from->DoSend(&p);
}

bool PacketManager::isEmpty(const Session* session)
{
	return session == nullptr;
}
