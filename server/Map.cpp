#include "pch.h"
#include "Map.h"
#include "Session.h"

void Section::AddClient(Session* session) {
    std::lock_guard<std::mutex> lock(_mutex);
    _clients.insert(session);
}

void Section::RemoveClient(Session* session) {
    std::lock_guard<std::mutex> lock(_mutex);
    _clients.erase(session);
}

SectionType Map::AddToSection(Session* session) {
    int x = session->getPosX();
    int y = session->getPosY();

    if (x > MAP_X_HALF && y > MAP_Y_HALF) {
        _sections[SectionType::RIGHTDOWN].AddClient(session);
        return SectionType::RIGHTDOWN;
    }
    else if (x > MAP_X_HALF && y <= MAP_Y_HALF) {
        _sections[SectionType::RIGHTUP].AddClient(session);
        return SectionType::RIGHTUP;
    }
    else if (x <= MAP_X_HALF && y > MAP_Y_HALF) {
        _sections[SectionType::LEFTDOWN].AddClient(session);
        return SectionType::LEFTDOWN;
    }
    else {
        _sections[SectionType::LEFTUP].AddClient(session);
        return SectionType::LEFTUP;
    }
}

void Map::RemoveFromSection(SectionType type, Session* session) {
    auto it = _sections.find(type);
    if (it != _sections.end()) {
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

