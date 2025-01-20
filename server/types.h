#pragma once
constexpr int PORT_NUM = 7777;
constexpr int BUF_SIZE = 1000;
constexpr int MAPSIZE = 1000;
constexpr int MAP_X_HALF = MAPSIZE / 2;
constexpr int MAP_Y_HALF = MAPSIZE / 2;



constexpr int MAX_CLIENT = 4000;
constexpr int MAX_NPC = 200000;

enum class SectionType : int {
    NONE = 0,
    SECTION1 = 1,
    SECTION2 = 2,
    SECTION3 = 3,
    SECTION4 = 4,
    SECTION5 = 5,
    SECTION6 = 6,
    SECTION7 = 7,
    SECTION8 = 8
};

using SessionRef = shared_ptr<class Session>;