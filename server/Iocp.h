#pragma once
#include"types.h"
#include"Over.h"
#include"Timer.h"
class Iocp
{
public:
	static Iocp& GetInstance()
	{
		static Iocp instance;
		return instance;
	}
	Iocp(const Iocp&) = delete;           
	Iocp& operator=(const Iocp&) = delete;
	Iocp(Iocp&&) = delete;                
	Iocp& operator=(Iocp&&) = delete;     
private:
	Iocp() {} 
	~Iocp() {}
public:
	bool networkSet(); 
	bool acceptStart();
	bool Register(SOCKET clientsocket,int id); 
	void dispatch(); 
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
};

