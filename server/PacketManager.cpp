#include "pch.h"
#include "PacketManager.h"
#include "packet.h"
#include "Map.h"
void PacketManager::processData(SessionRef client, char* packet)
{
	// 여기서 재조립 먼저 
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		client->setName(p->name);
		// 1 . 맵 섹션 분할 ( Map )
		sendLoginPacket(client);
		auto& instance = Map::GetInstance();
		SectionType roopsectiontype = instance.AddtoSection(client);

		client->_section = roopsectiontype;

		for (auto& cl : instance._sections[roopsectiontype]._clients)
		{
			if (cl == client)continue;
			sendAddPacket(client, cl); // 있는 애들한테 전부 전송 
			sendAddPacket(cl, client);
		}
		break;
	}
	case CS_MOVE_PLAYER:
	{
		CS_MOVE_PLAYER_PACKET* p = reinterpret_cast<CS_MOVE_PLAYER_PACKET*>(packet);

		client->Move(p->dir);
		// 플레이어 좌표가 경계를 넘어갔을경우? 변경 Change
		// 플레이어 좌표를 받았을 때 prevsection의 범위가 아닐경우? 변경 
		auto& instance = Map::GetInstance();
		SectionType roopsectiontype = instance.SectionCheck(client);
		for (auto& cl : instance._sections[roopsectiontype]._clients)
		{
			//if(can_see)
			sendMovePlayerPacket(cl,client);
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

void PacketManager::recycleData(SessionRef sclient, shared_ptr<Over> over, int num_bytes)
{
	SessionRef client = over->_client; // 이게 empty? 왜? 
	int remaindata = num_bytes + client->_prevremain;
	char* p = over->_buf;
	while (remaindata > 0)
	{
		int packetsize = p[0];
		if (packetsize <= remaindata)
		{
			// processpacket 이걸 어떻게 처리해줄까? 
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

void PacketManager::sendLoginPacket(SessionRef session)//여기서 섹션을 받아준다면? 
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

void PacketManager::sendAddPacket(SessionRef from, SessionRef to)
{
	//to -> from
	SC_ADD_PACKET p;
	p.size = sizeof(SC_ADD_PACKET);
	p.type = SC_ADD_OBJECT;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();

	from->DoSend(&p);
}

void PacketManager::sendMovePlayerPacket(SessionRef from, SessionRef to)
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.id = to->getId();
	p.x = to->getPosX();
	p.y = to->getPosY();
	p.left = to->_leftright;

	from->DoSend(&p);
}

void PacketManager::sendRemovePlayerPacket(SessionRef session)
{
}
