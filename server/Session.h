#pragma once
#include"types.h"
#include"Over.h"

class Session 
{
public:
	// 기본 생성자
	Session()
		: _clientsocket(INVALID_SOCKET),
		_prevremain(0),
		_manuallyDisconnected(false),
		_leftright(false),
		_section(SectionType::NONE),
		_id(-1),
		_x(0),
		_y(0),
		_hp(0),
		_exp(0),
		_att(0)
	{
		memset(_name, 0, sizeof(_name)); // _name 배열 초기화
	}
	void WorkerThread(Over* over, const DWORD& num_btytes);
	void DoRecv();
	void DoSend(void* packet);
	void disconnect() {
		_manuallyDisconnected = true;
		closesocket(_clientsocket); // 소켓 닫기
	}
	bool isDisconnected() const {
		// 1. 명시적으로 끊어진 상태
		if (_manuallyDisconnected) {
			return true;
		}

		// 2. 소켓 상태 확인
		if (_clientsocket == INVALID_SOCKET) {
			return true;
		}

		// 3. 비동기 작업 상태 확인 (예: Internal 값이 0이 아니면서 오류 발생)
		if (_over._over.Internal == 0) {
			// 비동기 작업이 펜딩 상태
			return false;
		}

		// 4. 추가적인 에러 상태 확인
		int error = 0;
		int len = sizeof(error);
		if (getsockopt(_clientsocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error != 0) {
			return true; // 소켓 에러 발생
		}

		return false; // 연결 상태가 정상
	}

public:
	int getId() { return _id; }
	short getPosX() { return _x; }
	short getPosY() { return _y; }
	int getHp() { return _hp; }
	int getExp() { return _exp; }
	int getAtt() { return _att; }
	int getMovetime() { return _movetime; }

	pair<short, short> getPairPos() { 
		pair<short, short> pp;
		pp.first = _x;
		pp.second = _y;
		return pp;
	}

	void setId(int id) { _id = id; }
	void setPosX(short x) { _x = x; }
	void setPosY(short y) { _y = y; }
	void setHp(int hp) { _hp = hp; }
	void setExp(int exp) { _exp = _exp; }
	void setAtt(int att) { _att = att; }
	void setName(char* name) { strcpy_s(_name, name); }
	void setMovetime(int movetime) { _movetime = movetime; }
public:
	void Move(int dir);
	bool NpcMove();
	int GetDistance(pair<short, short> pos);
	void ChasePlayer(Session* client);
	void Xmovecheck(pair<short,short> pos);
	void Ymovecheck(pair<short,short> pos);
	
public:
	Over _over;
	SOCKET _clientsocket;
	int _prevremain = 0;
	bool _manuallyDisconnected = false;
	bool _leftright = false;
	SectionType _section;
	mutex _lock;
	bool _isNpc;
	bool returncheck = false;
	atomic_bool _isalive = false;

public:
	unordered_set<Session*> _viewlist;
	unordered_set<Session*> _npcviewlist;
	
	mutex _viewlock;

private:
	int _id = -1;
	short _x, _y;
	int _hp;
	int _exp;
	int _att;
	int _movetime = 0;
	char _name[20];
};

class SessionManager {
public:
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}

	void CreateSession(int id, SOCKET socket);
	void CreateSession();
	void CreateNpc();

	void AddClient(int id);
	void RemoveClient(int id);
	void Dorecv(int id);
	void WorkerThread(int id, Over* over, const DWORD& numbytes);

private:
	SessionManager() = default;
	~SessionManager() = default;

	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;
public:
	pair<short, short> createRandomPos();

	array<Session, MAX_CLIENT> _clients;  
	array<Session, MAX_NPC> _npcs;
private:
	mutex _mutex; 
};

