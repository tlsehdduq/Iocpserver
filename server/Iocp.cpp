#include"pch.h"
#include "Iocp.h"
#include"PacketManager.h"
#include"Session.h"
#include"Map.h"

bool Iocp::networkSet()
{
	WSADATA wsaData;
	::WSAStartup(MAKEWORD(2, 2), &wsaData);
	_listensocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(PORT_NUM);

	if (SOCKET_ERROR == bind(_listensocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)))
	{
		int err = WSAGetLastError();
		cout << " Bind Error " << err << endl;
		return false;
	}
	if (SOCKET_ERROR == listen(_listensocket, SOMAXCONN))
	{
		cout << " Listen Error " << endl;
		return false;
	}
	if (_listensocket == INVALID_SOCKET)
	{
		cout << " Falied to Create listen socket " << endl;
		return false;
	}
	else
	{
		int optval;
		int optlen = sizeof(optval);
		if (getsockopt(_listensocket, SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&optval), &optlen) == SOCKET_ERROR)
		{
			cout << " Falied to get listen socket option " << endl;
			return false;
		}
		else
			cout << " Listen socket options retrieved successfully " << endl;

		sockaddr_in addr;
		int addrlen = sizeof(addr);
		if (getsockname(_listensocket, reinterpret_cast<sockaddr*>(&addr), &addrlen) == SOCKET_ERROR)
		{
			cout << "Failed to get local address of listen socket" << endl;
			return false;
		}
		else
			cout << " Listen socket is bound to port " << ntohs(addr.sin_port) << endl;
	}
	return true;
}

bool Iocp::acceptStart()
{
	SOCKADDR_IN client_addr;
	int addr_size = sizeof(client_addr);

	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(_listensocket), _iocpHandle, 9999, 0);

	if (_iocpHandle == NULL) {
		cout << "Failed to create IOCP" << endl;
		return false;
	}

	_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_clientsocket == INVALID_SOCKET) {
		cout << "Failed to create client socket" << endl;
		return false;
	}
	int flag = 1;
	setsockopt(_clientsocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	setsockopt(_clientsocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(flag));
	setsockopt(_clientsocket, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
	ZeroMemory(&_over._over, sizeof(_over._over));
	_over._type = CompType::Accept;

	BOOL ret = AcceptEx(_listensocket, _clientsocket, _over._buf, 0, addr_size + 16, addr_size + 16, 0, &_over._over);
	auto& mapinstance = Map::GetInstance();
	auto& instance = SessionManager::GetInstance();
	mapinstance.CreateObstacle();
	instance.CreateNpc();

	_timer.setIocpHandle(_iocpHandle);
	_timer.Run();

	if (ret == FALSE)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) {
			cout << "AcceptEx err " << err << endl;
			return false;
		}
	}
	else
		return true;
}

bool Iocp::Register(SOCKET clientsocket, int id)
{
	// key 값을 클라이언트 ID로 
	HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientsocket), _iocpHandle, id, 0);
	if (result == NULL) {
		int err = GetLastError();
		cout << " Iocp regist Err : " << err << endl;
		return false;
	}
	return true;
}

void Iocp::dispatch()
{
	DWORD numofBytes = 0;
	ULONG_PTR key;
	WSAOVERLAPPED* over = nullptr;
	while (true)
	{
		if (GetQueuedCompletionStatus(_iocpHandle, OUT & numofBytes, OUT & key, &over, INFINITE))
		{
			// 여기서 Session Dispatch 로 넘겨줘야함 
			Over* overex = reinterpret_cast<Over*>(over);
			if (overex->_type == CompType::Recv || overex->_type == CompType::Send)
			{
				auto& SessionManager = SessionManager::GetInstance();
				SessionManager.WorkerThread(key, overex, numofBytes);
			}
			else Workerthread(overex, numofBytes, key);
		}
	}
}

void Iocp::Workerthread(const Over* over, const DWORD& num_bytes, ULONG_PTR key) 
{
	switch (over->_type)
	{
	case CompType::Accept:
	{

		auto& Sessionmanager = SessionManager::GetInstance();
		int id = Sessionmanager.CreateID();
		Sessionmanager.CreateSession(id, _clientsocket); 
		Sessionmanager.Dorecv(id);
		// -- 초기화 
		_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		int flag = 1;
		setsockopt(_clientsocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		setsockopt(_clientsocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(flag));
		setsockopt(_clientsocket, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));

		ZeroMemory(&_over._over, sizeof(_over._over));
		int addrsize = sizeof(SOCKADDR_IN);

		BOOL result = AcceptEx(_listensocket, _clientsocket, _over._buf, 0, addrsize + 16, addrsize + 16, 0, &_over._over);

		if (!result && WSAGetLastError() != ERROR_IO_PENDING) {
			cout << "AcceptEx error: " << WSAGetLastError() << endl;
		}
	}
	break;
	//NPC EVENT 
	case CompType::NpcInit: 
	{
		auto& Sessionmanager = SessionManager::GetInstance();
		int npcid = over->_id;
		int clientid = key;
		PacketManager::sendNpcAddPacket(&Sessionmanager._clients[clientid], &Sessionmanager._npcs[npcid]);
		TimerEvent ev{ chrono::system_clock::now() + 1s,npcid,clientid,EVENT_TYPE::EV_NPC_MOVE };
		_timer.InitTimerQueue(ev);
		delete over;
		break;
	}
	case CompType::NpcMove: 
	{
		bool keep_alive = false;
		auto& Sessionmanager = SessionManager::GetInstance();
		int npcid = over->_id;
		int clientid = key; 
		if (Sessionmanager._npcs[npcid].NpcMove())
		{
			TimerEvent ev{ chrono::system_clock::now() + 1s,npcid,clientid,EVENT_TYPE::EV_NPC_MOVE };
			_timer.InitTimerQueue(ev);
		}

		delete over;
		break;
	}

	}
}




