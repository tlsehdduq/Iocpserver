#pragma once
#include"types.h"


class Session;
// Section class

struct PairHash {
	template <class T1, class T2>
	std::size_t operator()(const std::pair<T1, T2>& pair) const {
		auto h1 = std::hash<T1>{}(pair.first);
		auto h2 = std::hash<T2>{}(pair.second);
		return h1 ^ (h2 << 1);  // XOR 연산으로 해시값 결합
	}
};

class Section {
public:
	Section() = default;

	Section(Section&& other) noexcept {
		std::lock_guard<mutex> lock(other._mutex);
		_clients = std::move(other._clients);
		_npcs = std::move(other._npcs);
		obstacle = std::move(other.obstacle);
	}
	Section& operator=(Section&& other)noexcept {
		if (this != &other)
		{
			lock_guard<mutex>lock{ other._mutex };
			_clients = move(other._clients);
			_npcs = move(other._npcs);
			obstacle = move(other.obstacle);
		}
		return *this;
	}
	Section(const Section&) = delete;
	Section& operator=(const Section&) = delete; // mutex는 복사 대상 X  따라서 복사 생성자 연산자 삭제 

public:
	void AddClient(Session* session);
	void AddNpc(Session* session);
	void RemoveClient(Session* session);
	void RemoveNpc(Session* session);
	std::unordered_set<Session*> _clients;
	std::unordered_set<Session*> _npcs;

	std::unordered_set<pair<short, short>, PairHash> obstacle;  // 장애물  
private:
	std::mutex _mutex;
};

// Map class
class Map {
public:
	static Map& GetInstance() {
		static Map instance;
		return instance;
	}

	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;
	Map(Map&&) = delete;
	Map& operator=(Map&&) = delete;

	int AddToSection(Session* session);
	void RemoveFromSection(int type, Session* session);
	int SectionCheck(Session* session);
	bool CanSee(Session* to, Session* from) const;
	void NpcOn(Session* monster, Session* waker);
	void CreateObstacle();
	bool IsNearSectionBoundary(Session* client);
	vector<int> findnearsection(int sectionnum);
	vector<Session*> findnearnpc(int sectionnum);

	std::unordered_map<int, Section> _sections;
private:
	Map() {}
	~Map() {}
	mutex maplock;
	int _viewRange = 7;
	int _mapSize = 1000;
};