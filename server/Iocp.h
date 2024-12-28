#pragma once
#include"types.h"
#include"Over.h"
//#include"Session.h"
class Iocp
{
public:
	static Iocp& GetInstance()
	{
		static Iocp instance;
		return instance;
	}
	// ���� �� �̵� ����
	Iocp(const Iocp&) = delete;            // ���� ������ ����
	Iocp& operator=(const Iocp&) = delete; // ���� �Ҵ� ������ ����
	Iocp(Iocp&&) = delete;                 // �̵� ������ ����
	Iocp& operator=(Iocp&&) = delete;      // �̵� �Ҵ� ������ ����
private:
	Iocp() {} 
	~Iocp() {}
public:
	bool networkSet(); // network setting
	bool acceptStart();
	bool Register(SOCKET clientsocket,int id); // Iocp ��� 
	void dispatch(); // workerThread
	void Workerthread(const Over* over,const DWORD& num_bytes);
	
private:
	SOCKET _listensocket;
	SOCKET _clientsocket;
	HANDLE _iocpHandle;
	Over _over;
public:
	vector<thread> _threads;
	atomic<int> _clientid = 0;
};

