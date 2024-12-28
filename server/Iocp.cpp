#include"pch.h"
#include "Iocp.h"
#include"PacketManager.h"
#include"Session.h"
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
	_over._type = CompType::Accept;

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

bool Iocp::Register(SOCKET clientsocket, int id)
{
	// key ���� Ŭ���̾�Ʈ ID�� 
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
			// ���⼭ Session Dispatch �� �Ѱ������ 
			Over* overex = reinterpret_cast<Over*>(over);
			if (overex->_type != CompType::Accept)
			{
				auto& SessionManager = SessionManager::GetInstance();
				SessionManager.WorkerThread(key, overex, numofBytes);
			}
			else Workerthread(overex, numofBytes);
		}
	}
}

void Iocp::Workerthread(const Over* over, const DWORD& num_bytes) // ������ �߻� recv send�� ���� ����? overlapped ��� ���� ��� �ذ�? 
{
	switch (over->_type)
	{
	case CompType::Accept:
	{
		// ������� �ߵ��ٴ°���? �ٵ� Dorecv���������߻� ����� ���� ? Over , Socket 
		int id = _clientid;
		_clientid.fetch_add(1);
		auto& Sessionmanager = SessionManager::GetInstance();
		Sessionmanager.CreateSession(id, _clientsocket); // Session�ʱ�ȭ  socket ���� IO Regist 
		Sessionmanager.Dorecv(id);
		// -- �ʱ�ȭ 
		_clientsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		ZeroMemory(&_over._over, sizeof(_over._over));
		int addrsize = sizeof(SOCKADDR_IN);
		AcceptEx(_listensocket, _clientsocket, _over._buf, 0, addrsize + 16, addrsize + 16, 0, &_over._over);
	}
	break;
	// ������ timer 

	}
}


