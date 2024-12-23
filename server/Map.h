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

class Map // ��ֹ��� ���⼭ �־���� 
{
public:
	static Map& GetInstance()
	{
		static Map instance;
		return instance;
	}
	// ���� �� �̵� ����
	Map(const Map&) = delete;            // ���� ������ ����
	Map& operator=(const Map&) = delete; // ���� �Ҵ� ������ ����
	Map(Map&&) = delete;                 // �̵� ������ ����
	Map& operator=(Map&&) = delete;      // �̵� �Ҵ� ������ ����
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
	// section�� vector? set? map? map
	//array<Section, 4> _sections; // 0 : ���� ��, 1 : ���� �Ʒ�, 2 : ������ ��, 3: ������ �Ʒ� 
	mutex _mutex;
};

