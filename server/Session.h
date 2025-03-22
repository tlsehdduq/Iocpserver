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
		_section(-1),
		_id(-1),
		_x(0),
		_y(0),
		_hp(0),
		_exp(0),
		_att(0)
	{
		memset(_name, 0, sizeof(_name)); // _name 배열 초기화
	}
	Session(short x, short y) : _x{ x }, _y{ y }
	{

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
	int getLevel() { return _level; }
	char* getName() { return _name; }
	pair<short, short> getPairPos() { 
		pair<short, short> pos = { _x,_y };
		return pos;
	}


	void setId(int id) { _id = id; }
	void setPosX(short x) { _x = x; }
	void setPosY(short y) { _y = y; }
	void setHp(int hp) { _hp = hp; }
	void setExp(int exp) { _exp = _exp; }
	void setAtt(int att) { _att = att; }
	void setLevel(int level) { _level = level; }
	void setName(char* name) { strcpy_s(_name, name); }
	void setMovetime(int movetime) { _movetime = movetime; }
public:
	void Move(int dir);

public:
	bool NpcMove();
	void NpcAttack(Session* client);
	int GetDistance(pair<short, short> pos);
	void ChasePlayer(Session* client);
	bool MoveCheck(short& coord, short target, short othercoord, short otherTarget, bool isX);
	bool MoveInDir(short& coord, short target, short othercoord, int dir);
	void MoveDiagonally(short dx, short dy);

	void Xmovecheck(pair<short,short> pos);
	void Ymovecheck(pair<short,short> pos);
	
public:
	Over _over;
	SOCKET _clientsocket;
	int _prevremain = 0;
	bool _manuallyDisconnected = false;
	bool _leftright = false;
	int _section;
	mutex _lock;
	bool _isNpc;
	bool returncheck = false;
	bool _life = true;
	atomic_bool _isalive = false;
	atomic_bool _isAttack = false;
	int _monstercnt = 0;
	chrono::system_clock::time_point _attacktime;
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
	int _level = 0;
	char _name[20];
};

class SessionManager {
public:
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}
	int CreateID();
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

struct SessionPtrHash {
	std::size_t operator()(Session* session) const {
		return std::hash<short>()(session->getPosX()) ^ std::hash<short>()(session->getPosY());
	}
};
struct SessionPtrEqual {
	bool operator()(Session* lhs, Session* rhs) const {
		return lhs->getPosX() == rhs->getPosX() && lhs->getPosY() == rhs->getPosY();
	}
};