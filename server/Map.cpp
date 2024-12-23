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

void Section::BroadCastPacket(SessionRef session) //���� ��Ŷ�� �޴��� �޾ƾ� �� ��������� ������ ���⼭ �̷����µ� �׷� ��Ŷ�Ŵ����� �ʿ��ұ�? 
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

			cl->DoSend(&packet); // �ڱ��ڽ����׸� �����²� 
		}
		break;
	}
	case SectionType::RIGHTUP:
	{
		for (auto& cl : _sections[SectionType::RIGHTUP]._clients)
		{
			cl->DoSend(&packet); // �ڱ��ڽ����׸� �����²� 
		}
		break;
	}
	case SectionType::LEFTUP:
	{
		for (auto& cl : _sections[SectionType::LEFTUP]._clients)
		{
			cl->DoSend(&packet); // �ڱ��ڽ����׸� �����²� 
		}
		break;
	}
	case SectionType::LEFTDOWN:
	{
		for (auto& cl : _sections[SectionType::LEFTDOWN]._clients)
		{
			cl->DoSend(&packet); // �ڱ��ڽ����׸� �����²� 
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
		if (prevsection == SectionType::RIGHTDOWN) return SectionType::RIGHTDOWN; // ���� ������ ������ ���Ŀ� ���ٸ� �߰��� �ʿ� X 
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
