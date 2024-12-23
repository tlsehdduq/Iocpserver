#pragma once
#include"types.h"
#include"Map.h"
#include"Over.h"

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session() {}
	void WorkerThread(shared_ptr<Over> over, const DWORD& num_btytes);
	void DoRecv();
	void DoSend(void* packet);
	
public:
	int getId() { return _id; }
	short getPosX() { return _x; }
	short getPosY() { return _y; }
	int getHp() { return _hp; }
	int getExp() { return _exp; }
	int getAtt() { return _att; }

	void setId(int id) { _id = id; }
	void setPosX(short x) { _x = x; }
	void setPosY(short y) { _y = y; }
	void setHp(int hp) { _hp = hp; }
	void setExp(int exp) { _exp = _exp; }
	void setAtt(int att) { _att = att; }
	void setName(char* name) { strcpy_s(_name, name); }
public:
	void Move(int dir);
public:
	Over _over;
	SOCKET _clientsocket;
	int _prevremain = 0;
	bool _leftright = false;
	SectionType _section;
	mutex _lock;
private:
	int _id = -1;
	short _x, _y;
	int _hp;
	int _exp;
	int _att;
	//char _section = -1; // 0 :  (왼쪽위) 1 : (왼쪽아래) 2: (오른쪽 위) 3: (오른쪽 아래) 
	char _name[20];
};

class SessionManager {
public:
	// Singleton 패턴으로 생성 제한
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}

	SessionRef CreateSession(SOCKET socket);
	SessionRef GetSession(Session* session);

	void AddClient(SessionRef client);
	void RemoveClient(SessionRef client);


private:
	// 생성자/소멸자 비공개 (싱글톤 구현)
	SessionManager() = default;
	~SessionManager() = default;

	// 복사/이동 금지
	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;
public:
	int CreateClientId();
	pair<short, short> createRandomPos();
private:
	vector<SessionRef> _clients; // 전체 클라이언트 
	
	mutex _mutex; // 스레드 안전성 보장

};

