#include"pch.h"
#include "Iocp.h"
#include"PacketManager.h"
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

	_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	LINGER option;
	option.l_linger = 0;
	option.l_onoff = 1;
	setsockopt(_clientsocket, SOL_SOCKET, SO_LINGER, (const char*)&option, sizeof(option));

	auto& SessionManager = SessionManager::GetInstance();
	SessionRef session = SessionManager.CreateSession(_clientsocket); // session을 만들고 socket을 넘김
	session->setId(_clientid); // 초기 1번은 무조건 ID 0 
	_over._type = CompType::Accept;
	_over._client = session;

	BOOL ret = AcceptEx(_listensocket, _clientsocket, _over._buf, 0, addr_size + 16, addr_size + 16, 0, &_over._over);

	if (ret == FALSE)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) {
			cout << "AcceptEx err " << endl;
			return false;
		}
	}
	else
		return true;

}

bool Iocp::Register(SOCKET clientsocket, SessionRef session)
{
	ULONG_PTR key = reinterpret_cast<ULONG_PTR>(session.get());
	HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientsocket), _iocpHandle, key, 0);
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
	shared_ptr<Over> overex = make_shared<Over>();
	while (true)
	{
		if (GetQueuedCompletionStatus(_iocpHandle, OUT & numofBytes, OUT & key, reinterpret_cast<LPOVERLAPPED*>(&overex), INFINITE))
		{
			// 여기서 Session Dispatch 로 넘겨줘야함 
			if (overex->_type != CompType::Accept)
			{
				auto& SessionManager = SessionManager::GetInstance();
				auto rawSession = reinterpret_cast<Session*>(key);
				SessionRef session = SessionManager.GetSession(rawSession);

				overex->_client = session;
				overex->_client->WorkerThread(overex, numofBytes); // client가 NULL? key로 받아보자 
			}
			else Dispatch(overex, numofBytes);
		}
	}
}

void Iocp::Dispatch(shared_ptr<Over> over, const DWORD& num_bytes) // 복사비용 발생 recv send가 많을 수록? overlapped 비용 증가 어떻게 해결? 
{
	switch (over->_type)
	{
	case CompType::Accept:
	{
		// 여기까진 잘들어간다는거지? 근데 Dorecv에서문제발생 생기는 이유 ? Over , Socket 
		if (over->_client)
		{
			auto& SessionManager = SessionManager::GetInstance();
			SessionManager.AddClient(over->_client); // Iocp 등록 , 매니저 클라이언트에 추가, 
			over->_client->DoRecv(); // recv 등록 
			// -- 초기화 
			_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			_clientid.fetch_add(1);
			SessionRef resession = SessionManager.CreateSession(_clientsocket);
			resession->setId(_clientid);

			ZeroMemory(&_over, sizeof(_over));
			_over._client = resession;
			int addrsize = sizeof(SOCKADDR_IN);
			AcceptEx(_listensocket, _clientsocket, _over._buf, 0, addrsize + 16, addrsize + 16, 0, &_over._over);
		}
		else
			cout << " None Client " << endl;

	}
	break;

	}
}


