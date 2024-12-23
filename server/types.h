#pragma once
constexpr int PORT_NUM = 7777;
constexpr int BUF_SIZE = 1000;
constexpr int MAPSIZE = 1000;
constexpr int MAP_X_HALF = MAPSIZE / 2;
constexpr int MAP_Y_HALF = MAPSIZE / 2;

using SessionRef = shared_ptr<class Session>;