#pragma once
#include"types.h"

enum class SectionType : int
{
	LEFTUP = 0,
	LEFTDOWN = 1,
	RIGHTUP = 2,
	RIGHTDOWN = 3
};

class Section
{
public:
	void AddClient(SessionRef session);
	void RemoveClient(SessionRef session);
	void BroadCastPacket(SessionRef session);
	void setSectionType(SectionType sectionType) { _sectionType = sectionType; }
	SectionType getSectionType() { return _sectionType; }

	vector<SessionRef> _clients;
private:
	SectionType _sectionType;
};

class Map // 장애물도 여기서 넣어야함 
{
public:
	static Map& GetInstance()
	{
		static Map instance;
		return instance;
	}
	// 복사 및 이동 금지
	Map(const Map&) = delete;            // 복사 생성자 삭제
	Map& operator=(const Map&) = delete; // 복사 할당 연산자 삭제
	Map(Map&&) = delete;                 // 이동 생성자 삭제
	Map& operator=(Map&&) = delete;      // 이동 할당 연산자 삭제
	Map() {}
	~Map() {}
public:
	SectionType AddtoSection(SessionRef session);
	void RemovetoSection(SectionType type, SessionRef session);
	void BroadcastToSection(SessionRef session, void* packet, SectionType sectionType);
	SectionType SectionCheck(SessionRef session);
public:
	int mapsize = 1000;
	unordered_map<SectionType, Section> _sections; 
	// section별 vector? set? map? map
	//array<Section, 4> _sections; // 0 : 왼쪽 위, 1 : 왼쪽 아래, 2 : 오른쪽 위, 3: 오른쪽 아래 
	mutex _mutex;
};

