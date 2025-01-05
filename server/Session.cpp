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

void SessionManager::CreateNpc()
{
	auto& instance = Map::GetInstance();
	for (int i = MAX_CLIENT; i < MAX_NPC; ++i)
	{
		_clients[i].setId(i);
		_clients[i].setPosX(createRandomPos().first);
		_clients[i].setPosY(createRandomPos().second);
		_clients[i].setHp(100);
		_clients[i]._isNpc = true;

		_clients[i]._section = instance.AddToSection(&_clients[i]);
	}
}

// 클라이언트 추가
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
// 클라이언트 제거
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
		// 패킷 재조립도 여기서 해야함 , 그걸 packetmanager에서 해주자 
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

bool Session::NpcRandomMove()
{
	bool keepalive = false;
	auto& instance = Map::GetInstance();

	// Step 1: 시야 거리 안에 있는 플레이어 확인
	std::unordered_set<int> visibleClients;
	for (auto& cl : instance._sections[_section]._clients)
	{
		if (cl->_isNpc) continue;
		if (instance.CanSee(this, cl))
		{
			visibleClients.insert(cl->getId());
			keepalive = true;
		}
	}

	if (!keepalive)
	{
		_isalive = false;
		return _isalive;
	}

	// Step 2: NPC 랜덤 이동
	int dir = rand() % 4;
	switch (dir)
	{
	case 0: if (_y > 0) _y -= 1; break;
	case 1: if (_y < 1000) _y += 1; break;
	case 2: if (_x > 0) _x -= 1; break;
	case 3: if (_x < 1000) _x += 1; break;
	}

	instance.SectionCheck(this);

	// Step 3: 이동 후 시야 거리 다시 확인
	std::unordered_set<int> newVisibleClients;
	for (auto& cl : instance._sections[_section]._clients)
	{
		if (cl->_isNpc) continue;
		if (instance.CanSee(this, cl))
		{
			newVisibleClients.insert(cl->getId());
		}
	}

	// Step 4: 변화된 시야 거리 처리 (Add/Move/Remove 패킷 전송)
	for (int id : newVisibleClients)
	{
		if (visibleClients.count(id) == 0)
		{
			PacketManager::sendNpcUpdatePacket(&SessionManager::GetInstance()._clients[id], this);
		}
		else
		{
			PacketManager::sendNpcMovePacket(&SessionManager::GetInstance()._clients[id], this);
		}
	}

	for (int id : visibleClients)
	{
		if (newVisibleClients.count(id) == 0)
		{
			PacketManager::sendRemovePlayerPacket(&SessionManager::GetInstance()._clients[id], this);
		}
	}

	return _isalive;
}







