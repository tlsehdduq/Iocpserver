#include"pch.h"
#include "Session.h"
#include"Iocp.h"
#include"PacketManager.h"

SessionRef SessionManager::CreateSession(SOCKET socket) {
	if (socket == INVALID_SOCKET)
	{
		cout << " Create Session Invalid Socket " << endl;
		return nullptr;
	}
	auto session = std::make_shared<Session>();

	session->setPosX(createRandomPos().first);
	session->setPosY(createRandomPos().second);
	session->setExp(0);
	session->setHp(100);
	session->_clientsocket = socket;

	return session;
}
SessionRef SessionManager::GetSession(Session* session)
{
	std::lock_guard<mutex>lock(_mutex);
	for (const auto& client : _clients)
	{
		if (client.get() == session)
			return client;
	}
	cout << " No client info in manager" << endl; 
	return nullptr;
}
// 클라이언트 추가
void SessionManager::AddClient(SessionRef client) {

	if (!client || client->_clientsocket == INVALID_SOCKET) {
		std::cerr << "Add Client Invalid client or socket!" << std::endl;
		return;
	}
	auto& instance = Iocp::GetInstance();
	if (!instance.Register(client->_clientsocket, client))
	{
		cout << "register socket err " << endl;
	}

	std::lock_guard<std::mutex> lock(_mutex);
	_clients.emplace_back(client);
}
// 클라이언트 제거
void SessionManager::RemoveClient(SessionRef client) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = std::remove(_clients.begin(), _clients.end(), client);
	_clients.erase(it, _clients.end());
}

int SessionManager::CreateClientId()
{
	for (int i = 0; i < _clients.max_size(); ++i)
	{
		if (_clients[i]->getId() == -1)
		{
			return i;
		}
	}
	return -1;
}

pair<short, short> SessionManager::createRandomPos()
{
	std::random_device rd; // 고유한 시드를 위한 random_device
	std::default_random_engine dre{ rd() }; // random_device를 사용하여 초기화된 엔진
	std::uniform_int_distribution<int> uid{ 489, 499 };
	pair<short, short> _pos{ uid(dre),uid(dre) };

	return _pos;
}

void Session::WorkerThread(shared_ptr<Over> over, const DWORD& num_bytes)
{
	switch (over->_type)
	{
	case CompType::Recv:
	{
		// 패킷 재조립도 여기서 해야함 , 그걸 packetmanager에서 해주자 
		PacketManager::recycleData(shared_from_this(), over, num_bytes);
		break;
	}
	case CompType::Send:
	{
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
