#include "pch.h"
#include "PacketManager.h"
#include "Session.h"
#include "Map.h"

void PacketManager::processData(Session* client, char* packet)
{
	// 여기서 재조립 먼저 
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
			sendAddPacket(client, cl); // 있는 애들한테 전부 전송 
			sendAddPacket(cl, client);
		}
		break;
	}
	case CS_MOVE_PLAYER:
	{
		CS_MOVE_PLAYER_PACKET* p = reinterpret_cast<CS_MOVE_PLAYER_PACKET*>(packet);

		// 클라이언트 이동 처리
		client->setMovetime(p->move_time);
		client->Move(p->dir);

		// 현재 맵 인스턴스 가져오기
		auto& instance = Map::GetInstance();

		// 섹션 확인 및 갱신
		SectionType currentSection = instance.SectionCheck(client);

		// 현재 뷰 리스트와 새로 계산된 뷰 리스트 준비
		unordered_set<int> newViewList;
		unordered_set<int> oldViewList;

		// 기존 뷰 리스트 보호
		{
			lock_guard<mutex> lock(client->_viewlock);
			oldViewList = client->_viewlist; // 기존 뷰 리스트 복사
		}

		// 현재 섹션의 클라이언트 루프
		for (auto& session : instance._sections[currentSection]._clients)
		{
			// 본인을 제외
			if (session == client)
				continue;

			// 시야 범위 내 확인
			if (instance.CanSee(client, session))
			{
				newViewList.insert(session->getId()); // 새 뷰 리스트에 추가

				// 새로 보이는 클라이언트
				if (oldViewList.find(session->getId()) == oldViewList.end())
				{
					sendAddPacket(session,client); // 클라이언트에게 추가 패킷 전송
					sendAddPacket(client,session); // 상대방도 동일하게 처리
				}
				else
				{
					sendMovePlayerPacket(session,client); // 기존 뷰 리스트에 있는 경우 Move 패킷 전송
				}
			}
		}

		// 기존 뷰 리스트와 새 뷰 리스트 비교
		for (auto& id : oldViewList)
		{
			// 더 이상 보이지 않는 클라이언트
			if (newViewList.find(id) == newViewList.end())
			{
				sendRemovePlayerPacket(client,)
				client->sendRemovePacket(id, 1); // 클라이언트에게 제거 패킷 전송
				auto& session = _clients[id];
				session->sendRemovePacket(client->getId(), 1); // 상대방도 동일하게 처리
			}
		}

		// 뷰 리스트 갱신
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

void PacketManager::sendLoginPacket(Session* session)//여기서 섹션을 받아준다면? 
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
	// to 보낼 클라이언트 

	from->_viewlock.lock();
	from->_viewlist.insert(to->getId());
	from->_viewlock.unlock();

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
