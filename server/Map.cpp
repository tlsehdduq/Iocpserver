#include "pch.h"
#include "Map.h"
#include "Session.h"
#include"Iocp.h"
void Section::AddClient(Session* session) {
	std::lock_guard<std::mutex> lock(_mutex);
	_clients.insert(session);
}

void Section::AddNpc(Session* session)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_npcs.insert(session);
}

void Section::RemoveClient(Session* session) {
	std::lock_guard<std::mutex> lock(_mutex);
	_clients.erase(session);
}

void Section::RemoveNpc(Session* session)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_npcs.erase(session);
}


SectionType Map::AddToSection(Session* session) {
	int x = session->getPosX();
	int y = session->getPosY();

	if (x > MAP_X_HALF && y > MAP_Y_HALF) {
		if (session->_isNpc) {
			if (session->_section != SectionType::RIGHTDOWN)
				_sections[SectionType::RIGHTDOWN].AddNpc(session);
		}
		else
		{
			if (session->_section != SectionType::RIGHTDOWN)
				_sections[SectionType::RIGHTDOWN].AddClient(session);
		}
		return SectionType::RIGHTDOWN;
	}
	else if (x > MAP_X_HALF && y <= MAP_Y_HALF) {
		if (session->_isNpc)
		{
			if (session->_section != SectionType::RIGHTDOWN)
				_sections[SectionType::RIGHTUP].AddNpc(session);
		}
		else
		{
			if (session->_section != SectionType::RIGHTDOWN)
				_sections[SectionType::RIGHTUP].AddClient(session);
		}

		return SectionType::RIGHTUP;
	}
	else if (x <= MAP_X_HALF && y > MAP_Y_HALF) {
		if (session->_isNpc)
		{
			if (session->_section != SectionType::LEFTDOWN)
				_sections[SectionType::LEFTDOWN].AddNpc(session);
		}
		else
		{
			if (session->_section != SectionType::LEFTDOWN)
				_sections[SectionType::LEFTDOWN].AddClient(session);
		}
		return SectionType::LEFTDOWN;
	}
	else {
		if (session->_isNpc)
		{
			if (session->_section != SectionType::LEFTUP)
				_sections[SectionType::LEFTUP].AddNpc(session);
		}
		else
		{
			if (session->_section != SectionType::LEFTUP)
				_sections[SectionType::LEFTUP].AddClient(session);
		}
		return SectionType::LEFTUP;
	}
}

void Map::RemoveFromSection(SectionType type, Session* session) {
	auto it = _sections.find(type); 
	if (it != _sections.end()) {
		if (session->_isNpc)
			it->second.RemoveNpc(session);
		else
			it->second.RemoveClient(session);
	}
}

SectionType Map::SectionCheck(Session* session) {
	SectionType prevSection = session->_section; 
	SectionType newSection = AddToSection(session);

	if (prevSection != newSection) {
		RemoveFromSection(prevSection, session);
		session->_section = newSection;
	}

	return newSection;
}

bool Map::CanSee(Session* to, Session* from) const {
	int dx = std::abs(to->getPosX() - from->getPosX());
	int dy = std::abs(to->getPosY() - from->getPosY());
	return dx <= _viewRange && dy <= _viewRange;
}

void Map::NpcOn(Session* monster, Session* waker)
{
	if (monster->getHp() <= 0 || monster->_isalive == true)return;
	bool oldstate = false;
	if (false == atomic_compare_exchange_strong(&monster->_isalive, &oldstate, true))return;

	TimerEvent ev{ chrono::system_clock::now(),monster->getId(),waker->getId(),EVENT_TYPE::EV_INIT };

	auto& TimerInstance = Iocp::GetInstance();
	TimerInstance._timer.InitTimerQueue(ev);
}

void Map::CreateObstacle()
{
	std::random_device rd; // 고유한 시드를 위한 random_device
	std::default_random_engine dre{ rd() }; // random_device를 사용하여 초기화된 엔진
	std::uniform_int_distribution<int> uid_500{ 0, 500 };
	std::uniform_int_distribution<int> uid_1000{ 500, 1000 };

	for (int i = 0; i < 150; ++i)
	{
		_sections[SectionType::LEFTDOWN].obstacle.insert(uid_500(dre), uid_1000(dre));
		_sections[SectionType::LEFTUP].obstacle.insert(uid_500(dre), uid_500(dre));
		_sections[SectionType::RIGHTDOWN].obstacle.insert(uid_1000(dre), uid_1000(dre));
		_sections[SectionType::RIGHTUP].obstacle.insert(uid_1000(dre), uid_500(dre));
	}
}

