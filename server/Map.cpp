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
	std::vector<std::pair<short, short>> LUpos = {
	{222, 18}, {275, 14}, {178, 313}, {316, 277}, {327, 239},
	{119, 295}, {190, 357}, {495, 151}, {309, 295}, {332, 295},
	{370, 413}, {480, 288}, {260, 275}, {113, 174}, {59, 427},
	{157, 100}, {355, 397}, {323, 210}, {166, 117}, {60, 373},
	{303, 369}, {362, 408}, {463, 425}, {322, 178}, {38, 434},
	{237, 66}, {431, 133}, {40, 124}, {144, 228}, {315, 263},
	{268, 98}, {367, 167}, {146, 430}, {463, 293}, {432, 196},
	{491, 239}, {462, 202}, {255, 205}, {469, 341}, {418, 392},
	{478, 111}, {496, 127}, {491, 248}, {432, 483}, {400, 136},
	{23, 351}, {83, 391}, {432, 394}, {427, 242}, {146, 397},
	{164, 115}, {479, 311}, {331, 426}, {384, 369}, {378, 331},
	{273, 202}, {320, 362}, {477, 27}, {296, 342}, {84, 213},
	{176, 93}, {412, 21}, {126, 173}, {20, 396}, {198, 488},
	{312, 386}, {269, 155}, {482, 484}, {159, 47}, {102, 253},
	{314, 227}, {51, 194}, {355, 128}, {92, 58}, {71, 373},
	{208, 168}, {167, 375}, {439, 298}, {499, 394}, {60, 269},
	{134, 107}, {413, 234}, {250, 205}, {106, 349}, {376, 170},
	{135, 95}, {6, 124}, {82, 289}, {341, 487}, {230, 305},
	{309, 221}, {261, 328}, {29, 487}, {362, 483}, {373, 101},
	{8, 383}, {357, 222}, {368, 343}, {112, 161}, {243, 0}
	};
	std::vector<std::pair<short, short>> LDpos = {
		{10, 510}, {20, 520}, {30, 530}, {40, 540}, {50, 550},
		{60, 560}, {70, 570}, {80, 580}, {90, 590}, {100, 600},
		{110, 610}, {120, 620}, {130, 630}, {140, 640}, {150, 650},
		{160, 660}, {170, 670}, {180, 680}, {190, 690}, {200, 700},
		{210, 710}, {220, 720}, {230, 730}, {240, 740}, {250, 750},
		{260, 760}, {270, 770}, {280, 780}, {290, 790}, {300, 800},
		{310, 810}, {320, 820}, {330, 830}, {340, 840}, {350, 850},
		{360, 860}, {370, 870}, {380, 880}, {390, 890}, {400, 900},
		{410, 910}, {420, 920}, {430, 930}, {440, 940}, {450, 950},
		{460, 960}, {470, 970}, {480, 980}, {490, 990}, {500, 1000},
		{5, 505}, {15, 515}, {25, 525}, {35, 535}, {45, 545},
		{55, 555}, {65, 565}, {75, 575}, {85, 585}, {95, 595},
		{105, 605}, {115, 615}, {125, 625}, {135, 635}, {145, 645},
		{155, 655}, {165, 665}, {175, 675}, {185, 685}, {195, 695},
		{205, 705}, {215, 715}, {225, 725}, {235, 735}, {245, 745},
		{255, 755}, {265, 765}, {275, 775}, {285, 785}, {295, 795},
		{305, 805}, {315, 815}, {325, 825}, {335, 835}, {345, 845},
		{355, 855}, {365, 865}, {375, 875}, {385, 885}, {395, 895},
		{405, 905}, {415, 915}, {425, 925}, {435, 935}, {445, 945},
		{455, 955}, {465, 965}, {475, 975}, {485, 985}, {495, 995}
	};
	std::vector<std::pair<short, short>> RDpos = {
	{510, 510}, {520, 520}, {530, 530}, {540, 540}, {550, 550},
	{560, 560}, {570, 570}, {580, 580}, {590, 590}, {600, 600},
	{610, 610}, {620, 620}, {630, 630}, {640, 640}, {650, 650},
	{660, 660}, {670, 670}, {680, 680}, {690, 690}, {700, 700},
	{710, 710}, {720, 720}, {730, 730}, {740, 740}, {750, 750},
	{760, 760}, {770, 770}, {780, 780}, {790, 790}, {800, 800},
	{810, 810}, {820, 820}, {830, 830}, {840, 840}, {850, 850},
	{860, 860}, {870, 870}, {880, 880}, {890, 890}, {900, 900},
	{910, 910}, {920, 920}, {930, 930}, {940, 940}, {950, 950},
	{960, 960}, {970, 970}, {980, 980}, {990, 990}, {1000, 1000},
	{505, 505}, {515, 515}, {525, 525}, {535, 535}, {545, 545},
	{555, 555}, {565, 565}, {575, 575}, {585, 585}, {595, 595},
	{605, 605}, {615, 615}, {625, 625}, {635, 635}, {645, 645},
	{655, 655}, {665, 665}, {675, 675}, {685, 685}, {695, 695},
	{705, 705}, {715, 715}, {725, 725}, {735, 735}, {745, 745},
	{755, 755}, {765, 765}, {775, 775}, {785, 785}, {795, 795},
	{805, 805}, {815, 815}, {825, 825}, {835, 835}, {845, 845},
	{855, 855}, {865, 865}, {875, 875}, {885, 885}, {895, 895},
	{905, 905}, {915, 915}, {925, 925}, {935, 935}, {945, 945},
	{955, 955}, {965, 965}, {975, 975}, {985, 985}, {995, 995}
	};
	std::vector<std::pair<short, short>> RUpos = {
	{510, 10}, {520, 20}, {530, 30}, {540, 40}, {550, 50},
	{560, 60}, {570, 70}, {580, 80}, {590, 90}, {600, 100},
	{610, 110}, {620, 120}, {630, 130}, {640, 140}, {650, 150},
	{660, 160}, {670, 170}, {680, 180}, {690, 190}, {700, 200},
	{710, 210}, {720, 220}, {730, 230}, {740, 240}, {750, 250},
	{760, 260}, {770, 270}, {780, 280}, {790, 290}, {800, 300},
	{810, 310}, {820, 320}, {830, 330}, {840, 340}, {850, 350},
	{860, 360}, {870, 370}, {880, 380}, {890, 390}, {900, 400},
	{910, 410}, {920, 420}, {930, 430}, {940, 440}, {950, 450},
	{960, 460}, {970, 470}, {980, 480}, {990, 490}, {1000, 500},
	{505, 5}, {515, 15}, {525, 25}, {535, 35}, {545, 45},
	{555, 55}, {565, 65}, {575, 75}, {585, 85}, {595, 95},
	{605, 105}, {615, 115}, {625, 125}, {635, 135}, {645, 145},
	{655, 155}, {665, 165}, {675, 175}, {685, 185}, {695, 195},
	{705, 205}, {715, 215}, {725, 225}, {735, 235}, {745, 245},
	{755, 255}, {765, 265}, {775, 275}, {785, 285}, {795, 295},
	{805, 305}, {815, 315}, {825, 325}, {835, 335}, {845, 345},
	{855, 355}, {865, 365}, {875, 375}, {885, 385}, {895, 395},
	{905, 405}, {915, 415}, {925, 425}, {935, 435}, {945, 445},
	{955, 455}, {965, 465}, {975, 475}, {985, 485}, {995, 495}
	};

	_sections[SectionType::LEFTDOWN].obstacle.insert(LDpos.begin(), LDpos.end());
	_sections[SectionType::LEFTUP].obstacle.insert(LUpos.begin(), LUpos.end());
	_sections[SectionType::RIGHTDOWN].obstacle.insert(RDpos.begin(), RDpos.end());
	_sections[SectionType::RIGHTUP].obstacle.insert(RUpos.begin(), RUpos.end());
	
	cout << " obstalce Init complete" << endl; 
}

