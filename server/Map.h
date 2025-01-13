#pragma once
#include"types.h"


class Session;
// Section class
class Section {
public:
    void AddClient(Session* session);
    void AddNpc(Session* session);
    void RemoveClient(Session* session);
    void RemoveNpc(Session* session);


    std::unordered_set<Session*> _clients;
    std::unordered_set<Session*> _npcs;

    std::unordered_set<pair<short, short>> obstacle;  // ��ֹ�  


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

    // Deleted copy and move constructors
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

    std::unordered_map<SectionType, Section> _sections;
private:
    Map() {}
    ~Map() {}

    int _viewRange = 5;
    int _mapSize = 1000;
};