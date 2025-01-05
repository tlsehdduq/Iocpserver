#pragma once
#include"types.h"
#include"Over.h"
#include"Timer.h"
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
	void Workerthread(const Over* over,const DWORD& num_bytes, ULONG_PTR key);
	
private:
	SOCKET _listensocket;
	SOCKET _clientsocket;
	Over _over;

public:
	Timer _timer;
	HANDLE _iocpHandle;
	vector<thread> _threads;
	thread _timerthread;
	atomic<int> _clientid = 0;
};

