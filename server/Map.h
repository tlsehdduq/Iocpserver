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
    void AddClient(Session* session);
    void AddNpc(Session* session);
    void RemoveClient(Session* session);
    void RemoveNpc(Session* session);

    std::unordered_set<Session*> _clients;
    std::unordered_set<Session*> _npcs;

    std::unordered_set<pair<short, short>,PairHash> obstacle;  // 장애물  
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

    SectionType AddToSection(Session* session);
    void RemoveFromSection(SectionType type, Session* session);
    SectionType SectionCheck(Session* session);
    bool CanSee(Session* to, Session* from) const;
    void NpcOn(Session* monster,Session* waker);
    void CreateObstacle();
    bool IsNearSectionBoundary(Session* client);

    std::unordered_map<SectionType, Section> _sections;
private:
    Map()  {}
    ~Map() {}

    int _viewRange = 5;
    int _mapSize = 1000;
};