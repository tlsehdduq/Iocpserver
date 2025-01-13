#include"pch.h"
#include "Session.h"
#include"Iocp.h"
#include"PacketManager.h"
#include"Map.h"

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
	std::random_device rd; // ������ �õ带 ���� random_device
	std::default_random_engine dre{ rd() }; // random_device�� ����Ͽ� �ʱ�ȭ�� ����
	std::uniform_int_distribution<int> uid{ 0, 1000 };
	pair<short, short> _pos{ uid(dre),uid(dre) };

	return _pos;
}

void Session::WorkerThread(Over* over, const DWORD& num_bytes)
{
	switch (over->_type)
	{
	case CompType::Recv:
	{
		// ��Ŷ �������� ���⼭ �ؾ��� , �װ� packetmanager���� ������ 
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

bool Session::NpcMove()
{
	bool keepalive = false;
	auto& instance = Map::GetInstance();
	Session* TargetSession = nullptr;
	for (auto& cl : instance._sections[_section]._clients)
	{
		if (instance.CanSee(this, cl))
		{
			if (keepalive == false)
				keepalive = true;
			if (GetDistance(cl->getPairPos()) > 2)continue;
			if (TargetSession == nullptr)
				TargetSession = cl;
			else
			{
				if (GetDistance(TargetSession->getPairPos()) < GetDistance(cl->getPairPos()))
					TargetSession = cl;
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
		int dir = rand() % 4;

		switch (dir)
		{
		case 0: if (_y > 0) _y -= 1; break;
		case 1: if (_y < 1000) _y += 1; break;
		case 2: if (_x > 0) _x -= 1; break;
		case 3: if (_x < 1000) _x += 1; break;
		}

	}
	instance.SectionCheck(this);
	for (auto& cl : instance._sections[_section]._clients)
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
		//�÷��̾����� ���� 
		// Attack() 
	}
	if (dx < dy || dx == dy)
	{
		Xmovecheck(client->getPairPos());
	}
	else if (dx > dy)
	{
		Ymovecheck(client->getPairPos());
	}
}

//int Session::WhereisPlayer(short x, short y)
//{
//	int dir = -1;
//	if (_x > x && _y > y)
//	{
//		// ���� �� 
//		return 0;
//	}
//	else if (_x > x && _y < y)
//	{
//		// ���� �Ʒ� 
//		return 1;
//	}
//	else if (_x < x && _y > y)
//	{
//		// ������ �� 
//		return 2;
//	}
//	else if (_x < x && _y < y)
//	{
//		// ������ �Ʒ� 
//		return 3;
//	}
//}

void Session::Xmovecheck(pair<short, short> pos)
{
	pair<short, short> mypos;
	mypos.first = _x;
	mypos.second = _y;

	short myX = _x;
	short myY = _y;
	short targetX = pos.first;
	short targetY = pos.second;
	// �����϶� �� ������ ��ֹ� ��ġ �ľ� 
	// ��ֹ���ǥ�� ���� ������ ��ǥ�� ������? ��ȸ 
	auto& instance = Map::GetInstance();
	// ���縦 �ϴ°� ����? 
	if (myX == targetX)
	{
		if (myY == targetY) // ��ħ 
		{
			//attack();
		}
		else if (myY > targetY)
		{
			mypos.second--;
			auto it = instance._sections[_section].obstacle.find(mypos); //������� ���� 
			if (it == instance._sections[_section].obstacle.end())
				_y -= 1;
			else _x += 1; // �ٵ� �̷��� �ϸ�  ��� �Դ� ���ٸ� �Ұ��� ����غ����ҵ� 
		}
		else
		{
			_y += 1;
		}
	}
	else if (myX > targetX)
	{
		_x -= 1;
	}
	else
	{
		_x += 1;
	}
}

void Session::Ymovecheck(pair<short, short> pos)
{
	short myX = _x;
	short myY = _y;
	short targetX = pos.first;
	short targetY = pos.second;

	if (myY == targetY)
	{
		if (myX == targetX) // ��ħ 
		{
			//attack();
		}
		else if (myX > targetX)
		{
			_x -= 1;
		}
		else
		{
			_x += 1;
		}
	}
	else if (myY > targetY)
	{
		_y -= 1;
	}
	else
	{
		_y += 1;
	}
}





