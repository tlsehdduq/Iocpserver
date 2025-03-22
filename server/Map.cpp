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

int Map::AddToSection(Session* session) {

	int sectionX = session->getPosX() / (MAPSIZE / 50);
	int sectionY = session->getPosY() / (MAPSIZE / 50);

	int sectionIndex = sectionY * 50 + sectionX;
	int newSection = sectionIndex;

	if (session->_isNpc)
	{
		if (session->_section != newSection)
		{
			_sections[newSection].AddNpc(session);
		}
	}
	else
	{
		if (session->_section != newSection)
		{
			_sections[newSection].AddClient(session);
		}
	}

	return newSection;

}

void Map::RemoveFromSection(int type, Session* session) {
	auto it = _sections.find(type);
	if (it != _sections.end()) {
		if (session->_isNpc)
			it->second.RemoveNpc(session);
		else
			it->second.RemoveClient(session);
	}
}

int Map::SectionCheck(Session* session) {
	int prevSection = session->_section;
	int newSection = AddToSection(session);

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
	std::ifstream inFile("obstacles.txt");
	if (!inFile)
	{
		cout << " Failed obstacles load " << endl; 
	}
	random_device rd;
	default_random_engine dre{ rd() };
	uniform_int_distribution<int> uid{ 0, 999 };

	vector < pair<int, int>>obstacle; // vector?  탐색시간이 빠른걸로하는게 낫지않나? 
	int x, y;
	while (inFile >> x >> y)
	{
		obstacle.emplace_back(x, y);
	}

	//addsection 하는것처럼 하면됌 
	for (pair<int, int>obs : obstacle)
	{
		InitObstacle(obs);
	}

	cout << " obstalce Init complete" << endl;
}

void Map::InitObstacle(pair<int, int> pos)
{
	int posX = pos.first / (MAPSIZE / 50);
	int posY = pos.second / (MAPSIZE / 50);

	int sectionIndex = posY * 50 + posX;

	_sections[sectionIndex].obstacle.insert(pos);
}

bool Map::IsNearSectionBoundary(Session* client) {
	int posX = client->getPosX();
	int posY = client->getPosY();

	int sectionWidth = MAPSIZE / 50;
	int sectionHeight = MAPSIZE / 50;

	return (posX % sectionWidth <= 1 || sectionWidth - (posX % sectionWidth) <= 1 ||
		posY % sectionHeight <= 1 || sectionHeight - (posY % sectionHeight) <= 1);
}

vector<int> Map::findnearsection(int sectionnum)
{
	vector<int> nearsection;
	int offsets[] = { -1, 1,-50,50,49,-49,-51,51 };

	for (int offset : offsets) {
		int neighbor = sectionnum + offset;
		if (neighbor > 0 && neighbor < 2500) { 
			nearsection.emplace_back(neighbor);
		}
	}

	return nearsection;
}

