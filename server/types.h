#pragma once
constexpr int PORT_NUM = 7777;
constexpr int BUF_SIZE = 1000;
constexpr int MAPSIZE = 1000;
constexpr int MAP_X_HALF = MAPSIZE / 2;
constexpr int MAP_Y_HALF = MAPSIZE / 2;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 100;


constexpr int MAX_CLIENT = 6000;
constexpr int MAX_USER = 6000;
constexpr int MAX_NPC = 200000;


using SessionRef = shared_ptr<class Session>;