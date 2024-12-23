#include "pch.h"
#include "Session.h"
#include "Map.h"

void Section::AddClient(SessionRef session)
{
	_clients.emplace_back(session);
}

void Section::RemoveClient(SessionRef session)
{
	auto it = std::remove(_clients.begin(), _clients.end(), session);
	if (it != _clients.end()) {
		_clients.erase(it, _clients.end());
	}
}

void Section::BroadCastPacket(SessionRef session) //무슨 패킷을 받는지 받아야 함 결과적으로 전송은 여기서 이뤄지는데 그럼 패킷매니저가 필요할까? 
{

}

SectionType Map::AddtoSection(SessionRef session)
{
	lock_guard<mutex> lock(session->_lock);
	if (session->getPosX() > MAP_X_HALF && session->getPosY() > MAP_Y_HALF)
	{
		//RIGHT DOWN
		_sections[SectionType::RIGHTDOWN].AddClient(session);
		return SectionType::RIGHTDOWN;
	}
	else if (session->getPosX() > MAP_X_HALF && session->getPosY() < MAP_Y_HALF)
	{
		// RIGHT UP
		_sections[SectionType::RIGHTUP].AddClient(session);
		return SectionType::RIGHTUP;
	}
	else if (session->getPosX() < MAP_X_HALF && session->getPosY() > MAP_Y_HALF)
	{
		// LEFT DOWN
		_sections[SectionType::LEFTDOWN].AddClient(session);
		return SectionType::LEFTDOWN;
	}
	else
	{
		// LEFT UP 
		_sections[SectionType::LEFTUP].AddClient(session);
		return SectionType::LEFTUP;
	}
}

void Map::RemovetoSection(SectionType type, SessionRef session)
{
	lock_guard<mutex> lock(_mutex);

	switch (type)
	{
	case SectionType::LEFTDOWN:
	{
			_sections[SectionType::LEFTDOWN].RemoveClient(session);
		break;
	}
	case SectionType::LEFTUP:
	{
			_sections[SectionType::LEFTUP].RemoveClient(session);
		break;
	}
	case SectionType::RIGHTUP:
	{
			_sections[SectionType::RIGHTUP].RemoveClient(session);
		break;
	}
	case SectionType::RIGHTDOWN:
	{
			_sections[SectionType::RIGHTDOWN].RemoveClient(session);
		break;
	}
	}
}

void Map::BroadcastToSection(SessionRef session, void* packet, SectionType sectionType)
{
	switch (static_cast<SectionType>(sectionType))
	{
	case SectionType::RIGHTDOWN:
	{
		for (auto& cl : _sections[SectionType::RIGHTDOWN]._clients)
		{

			cl->DoSend(&packet); // 자기자신한테만 보내는꼴 
		}
		break;
	}
	case SectionType::RIGHTUP:
	{
		for (auto& cl : _sections[SectionType::RIGHTUP]._clients)
		{
			cl->DoSend(&packet); // 자기자신한테만 보내는꼴 
		}
		break;
	}
	case SectionType::LEFTUP:
	{
		for (auto& cl : _sections[SectionType::LEFTUP]._clients)
		{
			cl->DoSend(&packet); // 자기자신한테만 보내는꼴 
		}
		break;
	}
	case SectionType::LEFTDOWN:
	{
		for (auto& cl : _sections[SectionType::LEFTDOWN]._clients)
		{
			cl->DoSend(&packet); // 자기자신한테만 보내는꼴 
		}
		break;
	}
	}
}

SectionType Map::SectionCheck(SessionRef session)
{
	SectionType prevsection = session->_section;
	pair<short, short> sessionpos;
	sessionpos.first = session->getPosX();
	sessionpos.second = session->getPosY();

	if (sessionpos.first > MAP_X_HALF && sessionpos.second > MAP_Y_HALF)
	{
		//RIGHT DOWN
		if (prevsection == SectionType::RIGHTDOWN) return SectionType::RIGHTDOWN; // 기존 섹션이 움직인 이후와 같다면 추가할 필요 X 
		_sections[SectionType::RIGHTDOWN].AddClient(session);
		session->_section = SectionType::RIGHTDOWN;
		RemovetoSection(prevsection, session);
		return SectionType::RIGHTDOWN;
	}
	else if (sessionpos.first > MAP_X_HALF && sessionpos.second < MAP_Y_HALF)
	{
		// RIGHT UP
		if (prevsection == SectionType::RIGHTUP) 	return SectionType::RIGHTUP;
		_sections[SectionType::RIGHTUP].AddClient(session);
		session->_section = SectionType::RIGHTUP;
		RemovetoSection(prevsection, session);
		return SectionType::RIGHTUP;

	}
	else if (sessionpos.first < MAP_X_HALF && sessionpos.second > MAP_Y_HALF)
	{
		// LEFT DOWN
		if (prevsection == SectionType::LEFTDOWN) return SectionType::LEFTDOWN;
		_sections[SectionType::LEFTDOWN].AddClient(session);
		session->_section = SectionType::LEFTDOWN;
		RemovetoSection(prevsection, session);
		return SectionType::LEFTDOWN;
	}
	else
	{
		// LEFT UP 
		if (prevsection == SectionType::LEFTUP) return SectionType::LEFTUP;
		_sections[SectionType::LEFTUP].AddClient(session);
		session->_section = SectionType::LEFTUP;
		RemovetoSection(prevsection, session);
		return SectionType::LEFTUP;
	}


}
