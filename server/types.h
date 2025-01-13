#pragma once
constexpr int PORT_NUM = 7777;
constexpr int BUF_SIZE = 1000;
constexpr int MAPSIZE = 1000;
constexpr int MAP_X_HALF = MAPSIZE / 2;
constexpr int MAP_Y_HALF = MAPSIZE / 2;
constexpr int MAX_CLIENT = 5000;
constexpr int MAX_NPC = 200000;

enum class SectionType : int {
    NONE = 0,
    LEFTUP = 1,
    LEFTDOWN = 2,
    RIGHTUP = 3,
    RIGHTDOWN = 4
};

using SessionRef = shared_ptr<class Session>;