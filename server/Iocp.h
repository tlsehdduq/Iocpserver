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
	// 복사 및 이동 금지
	Iocp(const Iocp&) = delete;            // 복사 생성자 삭제
	Iocp& operator=(const Iocp&) = delete; // 복사 할당 연산자 삭제
	Iocp(Iocp&&) = delete;                 // 이동 생성자 삭제
	Iocp& operator=(Iocp&&) = delete;      // 이동 할당 연산자 삭제
private:
	Iocp() {} 
	~Iocp() {}
public:
	bool networkSet(); // network setting
	bool acceptStart();
	bool Register(SOCKET clientsocket,SessionRef session); // Iocp 등록 
	void dispatch(); // workerThread
	void Dispatch(shared_ptr<Over> over,const DWORD& num_bytes);
	void processData(SessionRef session,char* packet);
private:
	SOCKET _listensocket;
	SOCKET _clientsocket;
	HANDLE _iocpHandle;
	Over _over;
public:
	vector<thread> _threads;
	atomic<int> _clientid = 0;
};

