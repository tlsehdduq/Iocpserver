#include"pch.h"
#include "Session.h"
#include"Iocp.h"
#include"PacketManager.h"
#include"Map.h"

int SessionManager::CreateID()
{
	for (int i = 0; i < MAX_NPC; ++i)
	{
		lock_guard<mutex> ll{ _clients[i]._lock };
		if (_clients[i]._isalive == false)
		{
			return i;
		}
	}
	return -1;
}

void SessionManager::CreateSession(int id, SOCKET socket)
{
	if (id < 0 || id >= MAX_CLIENT)return;

	_clients[id].setPosX(createRandomPos().first);
	_clients[id].setPosY(createRandomPos().second);
	_clients[id].setExp(0);
	_clients[id].setHp(100);
	_clients[id].setId(id);
	_clients[id]._isNpc = false;
	_clients[id]._clientsocket = socket;

	AddClient(id);
}

void SessionManager::CreateSession()
{
	//for (int i = 0; i < MAX_CLIENT; ++i)
	//{
	//	_clients[i] = new Session;
	//}
}

void SessionManager::CreateNpc()
{
	auto& instance = Map::GetInstance();
	map<int, Section> sortsection;
	for (int i = 0; i < MAX_NPC; ++i)
	{
		//_npcs[i] = new Session;
		_npcs[i].setId(i);
		_npcs[i].setPosX(createRandomPos().first);
		_npcs[i].setPosY(createRandomPos().second);
		_npcs[i].setHp(100);
		_npcs[i]._isNpc = true;
		_npcs[i]._section = instance.AddToSection(&_npcs[i]);
	}



	cout << " Npc Setting Complete" << endl;
}

void SessionManager::AddClient(int id) {

	if (_clients[id]._clientsocket == INVALID_SOCKET) {
		std::cerr << "Add Client Invalid client or socket!" << std::endl;
		return;
	}
	auto& instance = Iocp::GetInstance();
	if (!instance.Register(_clients[id]._clientsocket, id))
	{
		cout << "register socket err " << endl;
	}
}

void SessionManager::RemoveClient(int id) {
	std::lock_guard<std::mutex> lock(_mutex);
	_clients[id].setId(-1);
}

void SessionManager::Dorecv(int id)
{
	_clients[id].DoRecv();
}

void SessionManager::WorkerThread(int id, Over* over, const DWORD& numbytes)
{
	_clients[id].WorkerThread(over, numbytes);
}

pair<short, short> SessionManager::createRandomPos()
{
	std::random_device rd; // 고유한 시드를 위한 random_device
	std::default_random_engine dre{ rd() }; // random_device를 사용하여 초기화된 엔진
	std::uniform_int_distribution<int> uid{ 0, 999 };
	pair<short, short> _pos{ uid(dre),uid(dre) };

	return _pos;
}

void Session::WorkerThread(Over* over, const DWORD& num_bytes)
{
	switch (over->_type)
	{
	case CompType::Recv:
	{
		PacketManager::recycleData(this, over, num_bytes);
		break;
	}
	case CompType::Send:
	{
		delete over;
		break;
	}
	}
}

void Session::DoRecv()
{
	DWORD flag = 0;
	memset(&_over._over, 0, sizeof(_over._over));
	_over._wsaBuf.len = BUF_SIZE - _prevremain;
	_over._wsaBuf.buf = _over._buf + _prevremain;
	int result = WSARecv(_clientsocket, &_over._wsaBuf, 1, 0, &flag, &_over._over, 0);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
			cout << " Recv Error " << err << endl;
	}
}

void Session::DoSend(void* packet)
{
	Over* sendover = new Over{ reinterpret_cast<char*>(packet) };
	WSASend(_clientsocket, &sendover->_wsaBuf, 1, 0, 0, &sendover->_over, 0);
}

void Session::Move(int dir)
{
	switch (dir)
	{
	case 0:
	{
		_y -= 1;
		break;
	}
	case 1:
	{
		_y += 1;
		break;
	}
	case 2:
	{
		_x -= 1;
		_leftright = true;
		break;
	}
	case 3:
	{
		_x += 1;
		_leftright = false;
		break;
	}
	}
}

void Session::NpcAttack(Session* client)
{
	auto& instance = SessionManager::GetInstance();
	for (auto& cl : instance._clients)
	{
		PacketManager::sendNpcAttackPacket(this, &cl,client);
	}
	client->setHp(client->getHp() - 10);
}

bool Session::NpcMove()
{
	if (_isalive == false) return false;
	bool keepalive = false;
	auto& instance = Map::GetInstance();
	int closestDistance = 9999;
	Session* TargetSession = nullptr;
	unordered_set<Session*> sectionclients;
	for (auto& cl : instance._sections[_section]._clients)
	{
		if (instance.CanSee(this, cl))
		{
			if (!keepalive)
				keepalive = true;
			int distance = GetDistance(cl->getPairPos());
			if (distance > 2)
				continue;
			if (TargetSession == nullptr || distance < closestDistance)
			{
				TargetSession = cl;
				closestDistance = distance;
			}
		}
	}
	if (!keepalive)
	{
		_isalive = false;
		return _isalive;
	}
	if (TargetSession != nullptr)
		ChasePlayer(TargetSession);
	else
	{
		std::array<std::pair<int, int>, 4> directions = {
		std::make_pair(0, -1),
		std::make_pair(0, 1),
		std::make_pair(-1, 0),
		std::make_pair(1, 0)
		};
		int dir = rand() % 4;
		int dx = directions[dir].first;
		int dy = directions[dir].second;
		short newX = _x + dx;
		short newY = _y + dy;

		if (newX >= 0 && newX <= 1000 && newY >= 0 && newY <= 1000) {
			std::pair<short, short> newPos = { newX, newY };

			if (instance._sections[_section].obstacle.find(newPos) == instance._sections[_section].obstacle.end()) {
				_x = newX;
				_y = newY;
			}
		}
	}

	int posX = _x;
	int posY = _y;
	int mapXHalfDiv2 = MAP_X_HALF / 2;
	int mapYHalf = MAP_Y_HALF;

	if (instance.IsNearSectionBoundary(this))
	{
		instance.SectionCheck(this);
	}
	{
		lock_guard<mutex> sectionlock{ _lock };
		sectionclients = instance._sections[_section]._clients;
	}

	vector<int> nearsection = instance.findnearsection(_section);

	for (auto& cl : sectionclients)
	{
		if (instance.CanSee(this, cl))
		{
			PacketManager::sendNpcMovePacket(cl, this);
		}
	}
	unordered_set<Session*> nearsectionclients;
	for (int section : nearsection)
	{
		lock_guard<mutex> sectionlock{ _lock }; // lock
		nearsectionclients.insert(instance._sections[section]._clients.begin(), instance._sections[section]._clients.end());
	}

	for (auto cl : nearsectionclients)
	{
		if (instance.CanSee(this, cl))
		{
			PacketManager::sendNpcMovePacket(cl, this);
		}
	}


	return _isalive;
}

int Session::GetDistance(pair<short, short> pos)
{
	short myX = _x;
	short myY = _y;

	short targetX = pos.first;
	short targetY = pos.second;

	int dis = std::abs(myX - targetX) + std::abs(myY - targetY);

	return dis;
}

void Session::ChasePlayer(Session* client)
{
	short x = client->getPosX();
	short y = client->getPosY();

	int dx = std::abs(x - _x);
	int dy = std::abs(y - _y);

	if (dx == 0 && dy == 0)
	{
		if (_isAttack == true)
		{
			auto time = chrono::system_clock::now();
			if ((time - _attacktime) > 3s)
			{
				_isAttack = false;
				return;
			}
			else return; 
		}
		bool oldstate = false;
		if (false == atomic_compare_exchange_strong(&_isAttack, &oldstate, true))return;
		_attacktime = chrono::system_clock::now();
	
		NpcAttack(client);
		return;
	}
	if (dx < dy || dx == dy)
	{
		_isAttack = false;
		Xmovecheck(client->getPairPos());
	}
	else
	{
		_isAttack = false;
		Ymovecheck(client->getPairPos());
	}
}

bool Session::MoveCheck(short& coord, short target, short othercoord, short otherTarget, bool isX)
{
	auto& instance = Map::GetInstance();
	short step = (target > coord) ? 1 : -1;

	// 기본 방향 이동 시도
	pair<short, short> temp = isX
		? pair<short, short>(coord + step, othercoord)
		: pair<short, short>(othercoord, coord + step);

	if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
	{
		coord += step;
		return true;
	}

	// 대각선 방향 이동 시도
	short diagStepX = (otherTarget > othercoord) ? 1 : -1;
	pair<short, short> diagTemp = isX
		? pair<short, short>(coord + step, othercoord + diagStepX)
		: pair<short, short>(othercoord + diagStepX, coord + step);

	if (instance._sections[_section].obstacle.find(diagTemp) == instance._sections[_section].obstacle.end())
	{
		coord += step;
		othercoord += diagStepX;
		return true;
	}

	return false;
}

bool Session::MoveInDir(short& coord, short target, short othercoord, int dir)
{
	auto& instance = Map::GetInstance();
	short step = (target > coord) ? 1 : -1;
	pair<short, short> temp = (dir == 0) ? pair<short, short>(coord + step, othercoord) : pair<short, short>(othercoord, coord + step);

	if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
	{
		coord += step;
		return true;
	}
	return false;
}

void Session::MoveDiagonally(short dx, short dy)
{
	auto& instance = Map::GetInstance();
	pair<short, short> temp = { _x + dx, _y + dy };
	if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
	{
		_x += dx;
		_y += dy;
	}
}

void Session::Xmovecheck(pair<short, short> pos)
{
	pair<short, short> mypos;
	mypos.first = _x;
	mypos.second = _y;

	short myX = _x;
	short myY = _y;
	short targetX = pos.first;
	short targetY = pos.second;
	// 움직일때 내 섹션의 장애물 위치 파악 
	auto& instance = Map::GetInstance();

	if (myX == targetX) // X 절대값이 같으면? Y축 이동 
	{
		if (myY > targetY)
		{
			pair<short, short> temp = { _x,_y - 1 }; // 가려는 - 방향에 장애물좌표 있다면? YmoveCheck이 아니라 
			if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
				_y -= 1;
			else
			{
				//해당 이동경로에 장애물이 있는상황 
				_x -= 1;
				_y -= 1;
			}
		}
		else
		{
			pair<short, short> temp = { _x,_y + 1 };
			if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
				_y += 1;
			else
			{
				//해당 이동경로에 장애물이 있는상황 
				_x -= 1;
				_y += 1;
			}
		}
	}
	else if (myX > targetX)
	{
		pair<short, short> temp = { _x - 1,_y };
		if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
			_x -= 1;
		else
		{
			//해당 이동경로에 장애물이 있는상황 
			_x -= 1;
			_y += 1;
		}

	}
	else
	{
		pair<short, short> temp = { _x + 1,_y };
		if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
			_x += 1;
		else
		{
			//해당 이동경로에 장애물이 있는상황 
			_x += 1;
			_y += 1;
		}
	}
}

void Session::Ymovecheck(pair<short, short> pos)
{
	short myX = _x;
	short myY = _y;
	short targetX = pos.first;
	short targetY = pos.second;
	auto& instance = Map::GetInstance();
	if (myY == targetY)
	{
		if (myX == targetX) // 겹침 
		{
			//attack();
		}
		else if (myX > targetX)
		{
			pair<short, short> temp = { _x - 1,_y };
			if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
				_x -= 1;
			else
			{
				//해당 이동경로에 장애물이 있는상황 
				_x -= 1;
				_y -= 1;
			}
		}
		else
		{
			pair<short, short> temp = { _x + 1,_y };
			if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
				_x += 1;
			else
			{
				//해당 이동경로에 장애물이 있는상황 
				_x += 1;
				_y -= 1;
			}
		}
	}
	else if (myY > targetY)
	{
		pair<short, short> temp = { _x,_y - 1 };
		if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
			_y -= 1;
		else
		{
			//해당 이동경로에 장애물이 있는상황 
			_x -= 1;
			_y -= 1;
		}
	}
	else
	{
		pair<short, short> temp = { _x,_y + 1 };
		if (instance._sections[_section].obstacle.find(temp) == instance._sections[_section].obstacle.end())
			_y += 1;
		else
		{
			//해당 이동경로에 장애물이 있는상황 
			_x -= 1;
			_y += 1;
		}
	}
}





