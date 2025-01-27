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
	//int x = session->getPosX();
	//int y = session->getPosY();

	//if (x >= MAP_X_HALF + (MAP_X_HALF/2) && y < MAP_Y_HALF) {
	//	if (session->_isNpc) {
	//		if (session->_section != SectionType::SECTION4)
	//			_sections[SectionType::SECTION4].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION4)
	//			_sections[SectionType::SECTION4].AddClient(session);
	//	}
	//	return SectionType::SECTION4;
	//}
	//else if (x >= MAP_X_HALF && x < MAP_X_HALF + (MAP_X_HALF/2)  && y < MAP_Y_HALF) {
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION3)
	//			_sections[SectionType::SECTION3].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION3)
	//			_sections[SectionType::SECTION3].AddClient(session);
	//	}

	//	return SectionType::SECTION3;
	//}
	//else if (x >= MAP_X_HALF/2 && x < MAP_X_HALF && y < MAP_Y_HALF) {
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION2)
	//			_sections[SectionType::SECTION2].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION2)
	//			_sections[SectionType::SECTION2].AddClient(session);
	//	}
	//	return SectionType::SECTION2;
	//}
	//else if( x >= 0&& x < (MAP_X_HALF/2) && y < MAP_Y_HALF){
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION1)
	//			_sections[SectionType::SECTION1].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION1)
	//			_sections[SectionType::SECTION1].AddClient(session);
	//	}
	//	return SectionType::SECTION1;
	//}
	//else if (x >= 0 && x <= (MAP_X_HALF / 2) && y >= MAP_Y_HALF)
	//{
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION5)
	//			_sections[SectionType::SECTION5].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION5)
	//			_sections[SectionType::SECTION5].AddClient(session);
	//	}
	//	return SectionType::SECTION5;
	//}
	//else if (x >=(MAP_X_HALF / 2) && x <= MAP_X_HALF && y >= MAP_Y_HALF)
	//{
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION6)
	//			_sections[SectionType::SECTION6].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION6)
	//			_sections[SectionType::SECTION6].AddClient(session);
	//	}
	//	return SectionType::SECTION6;
	//}
	//else if (x >= MAP_X_HALF && x <= MAP_X_HALF + (MAP_X_HALF / 2) && y >= MAP_Y_HALF)
	//{
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION7)
	//			_sections[SectionType::SECTION7].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION7)
	//			_sections[SectionType::SECTION7].AddClient(session);
	//	}
	//	return SectionType::SECTION7;
	//}
	//else
	//{
	//	if (session->_isNpc)
	//	{
	//		if (session->_section != SectionType::SECTION8)
	//			_sections[SectionType::SECTION8].AddNpc(session);
	//	}
	//	else
	//	{
	//		if (session->_section != SectionType::SECTION8)
	//			_sections[SectionType::SECTION8].AddClient(session);
	//	}
	//	return SectionType::SECTION8;
	//}
	// Calculate the section index based on x and y coordinates
	// Calculate the section index based on x and y coordinates
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
	vector<pair<short, short>> Section1obs =
	{
		{10, 20}, {50, 100}, {200, 300}, {150, 400}, {230, 120},
	{5, 250}, {60, 50}, {125, 375}, {210, 490}, {240, 10},
	{20, 220}, {100, 150}, {180, 450}, {90, 350}, {30, 40},
	{50, 80}, {220, 410}, {150, 300}, {200, 100}, {75, 275},
	{15, 20}, {60, 30}, {190, 460}, {140, 250}, {200, 60},
	{25, 240}, {110, 360}, {180, 70}, {220, 290}, {40, 50},
	{60, 170}, {120, 390}, {210, 110}, {230, 400}, {80, 300},
	{20, 10}, {150, 250}, {190, 50}, {70, 370}, {90, 400},
	{220, 120}, {130, 430}, {100, 200}, {140, 50}, {230, 360},
	{40, 450}, {180, 30}, {120, 70}, {200, 270}, {10, 40}
	};
	vector<pair<short, short>> Section2obs =
	{
		{260, 20}, {300, 100}, {450, 200}, {400, 300}, {490, 400},
	{270, 250}, {360, 50}, {425, 375}, {310, 450}, {280, 10},
	{290, 220}, {350, 150}, {480, 450}, {390, 350}, {330, 40},
	{250, 80}, {470, 410}, {350, 300}, {400, 100}, {375, 275},
	{275, 20}, {460, 30}, {390, 460}, {440, 250}, {300, 60},
	{320, 240}, {410, 360}, {380, 70}, {420, 290}, {440, 50},
	{260, 170}, {320, 390}, {410, 110}, {450, 400}, {380, 300},
	{300, 10}, {450, 250}, {490, 50}, {370, 370}, {290, 400},
	{420, 120}, {350, 430}, {300, 200}, {440, 50}, {460, 360},
	{330, 450}, {380, 30}, {420, 70}, {450, 270}, {250, 40}
	};
	vector<pair<short, short>> Section3obs =
	{
 {510, 20}, {550, 100}, {700, 200}, {650, 300}, {740, 400},
	{520, 250}, {610, 50}, {675, 375}, {560, 450}, {580, 10},
	{590, 220}, {620, 150}, {730, 450}, {680, 350}, {630, 40},
	{550, 80}, {670, 410}, {620, 300}, {690, 100}, {575, 275},
	{515, 20}, {660, 30}, {690, 460}, {640, 250}, {500, 60},
	{620, 240}, {710, 360}, {580, 70}, {620, 290}, {740, 50},
	{560, 170}, {620, 390}, {610, 110}, {750, 400}, {680, 300},
	{600, 10}, {650, 250}, {590, 50}, {570, 370}, {690, 400},
	{620, 120}, {750, 430}, {500, 200}, {640, 50}, {730, 360},
	{530, 450}, {580, 30}, {620, 70}, {750, 270}, {500, 40}
	};
	vector<pair<short, short>> Section4obs =
	{
		{760, 20}, {800, 100}, {950, 200}, {850, 300}, {990, 400},
	{770, 250}, {860, 50}, {925, 375}, {810, 450}, {880, 10},
	{790, 220}, {850, 150}, {980, 450}, {890, 350}, {830, 40},
	{750, 80}, {970, 410}, {850, 300}, {900, 100}, {875, 275},
	{775, 20}, {960, 30}, {890, 460}, {940, 250}, {800, 60},
	{820, 240}, {910, 360}, {880, 70}, {920, 290}, {940, 50},
	{760, 170}, {820, 390}, {910, 110}, {950, 400}, {880, 300},
	{800, 10}, {950, 250}, {990, 50}, {970, 370}, {790, 400},
	{920, 120}, {850, 430}, {800, 200}, {940, 50}, {970, 360},
	{830, 450}, {880, 30}, {920, 70}, {950, 270}, {750, 40}
	};
	vector<pair<short, short>> Section5obs =
	{
		  {20, 510}, {50, 600}, {120, 720}, {180, 840}, {230, 910},
	{10, 530}, {90, 650}, {140, 750}, {200, 890}, {250, 970},
	{30, 560}, {70, 640}, {160, 760}, {190, 830}, {220, 920},
	{50, 590}, {130, 710}, {170, 800}, {210, 850}, {240, 980},
	{60, 520}, {100, 630}, {190, 770}, {200, 820}, {230, 930},
	{40, 580}, {80, 720}, {150, 810}, {180, 860}, {240, 910},
	{20, 610}, {110, 690}, {120, 730}, {140, 790}, {230, 850},
	{30, 550}, {130, 650}, {170, 830}, {210, 870}, {220, 940},
	{10, 600}, {50, 740}, {160, 820}, {180, 890}, {250, 960}
	};
	vector<pair<short, short>> Section6obs =
	{
		{270, 510}, {300, 620}, {320, 740}, {400, 820}, {490, 910},
	{280, 530}, {350, 650}, {390, 750}, {410, 890}, {470, 980},
	{300, 540}, {320, 660}, {360, 760}, {440, 850}, {480, 920},
	{260, 510}, {370, 610}, {420, 790}, {430, 830}, {490, 910},
	{310, 520}, {340, 630}, {400, 750}, {420, 820}, {470, 890},
	{290, 590}, {370, 720}, {380, 810}, {440, 860}, {480, 910},
	{270, 650}, {310, 710}, {330, 770}, {360, 830}, {490, 860},
	{300, 550}, {380, 640}, {400, 730}, {410, 810}, {460, 940}
	};
	vector<pair<short, short>> Section7obs =
	{
		 {510, 510}, {600, 600}, {700, 720}, {650, 850}, {740, 900},
	{520, 530}, {610, 670}, {675, 780}, {560, 840}, {690, 930},
	{730, 590}, {620, 640}, {710, 780}, {660, 890}, {600, 920},
	{550, 510}, {670, 690}, {690, 790}, {680, 870}, {590, 910},
	{610, 520}, {700, 610}, {600, 750}, {710, 830}, {740, 880},
	{510, 600}, {610, 720}, {700, 830}, {720, 890}, {690, 910},
	{600, 650}, {640, 750}, {660, 820}, {700, 850}, {730, 860},
	{510, 550}, {590, 640}, {620, 770}, {700, 810}, {720, 940}
	};
	vector<pair<short, short>> Section8obs =
	{
		{760, 510}, {800, 620}, {850, 740}, {950, 820}, {900, 910},
	{770, 530}, {860, 650}, {875, 780}, {960, 890}, {880, 980},
	{800, 590}, {850, 640}, {930, 750}, {880, 850}, {970, 920},
	{760, 510}, {860, 710}, {900, 780}, {920, 830}, {970, 910},
	{800, 520}, {850, 630}, {890, 750}, {910, 820}, {960, 890},
	{780, 590}, {860, 720}, {900, 810}, {920, 860}, {980, 910},
	{750, 650}, {810, 720}, {830, 770}, {880, 830}, {930, 860},
	{780, 550}, {860, 640}, {910, 730}, {920, 810}, {950, 940}
	};

	//_sections[SectionType::SECTION1].obstacle.insert(Section1obs.begin(), Section1obs.end());
	//_sections[SectionType::SECTION2].obstacle.insert(Section2obs.begin(), Section2obs.end());
	//_sections[SectionType::SECTION3].obstacle.insert(Section3obs.begin(), Section3obs.end());
	//_sections[SectionType::SECTION4].obstacle.insert(Section4obs.begin(), Section4obs.end());
	//_sections[SectionType::SECTION5].obstacle.insert(Section5obs.begin(), Section5obs.end());
	//_sections[SectionType::SECTION6].obstacle.insert(Section6obs.begin(), Section6obs.end());
	//_sections[SectionType::SECTION7].obstacle.insert(Section7obs.begin(), Section7obs.end());
	//_sections[SectionType::SECTION8].obstacle.insert(Section8obs.begin(), Section8obs.end());

	cout << " obstalce Init complete" << endl;
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
		if (neighbor > 0 && neighbor < 2500) { // 유효한 섹션 번호인지 확인
			// Index만 따로 받아와서 직접접근하는건? lock비용도 안들고 그래도될거같은데? 
			nearsection.emplace_back(neighbor);
		}
	}

	return nearsection;
}

vector<Session*> Map::findnearnpc(int sectionnum)
{
	vector<Session*> nearsectionclients;
	vector<int> offsets = { -1, +1, -50, +50, -49, +49, -51, +51 };

	lock_guard<mutex> ll{ maplock }; // 읽기 작업에 필요한 최소한의 잠금

	for (int offset : offsets) {
		int neighbor = sectionnum + offset;
		if (neighbor >= 0 && neighbor < 1000) { // 유효한 섹션 번호인지 확인
			//for(auto& cl : _sections[neighbor]._npcs)
			//nearsectionclients.emplace_back(_sections[neighbor]._npcs.begin(), _sections[neighbor]._npcs.end());
		}
	}

	return nearsectionclients;
}
