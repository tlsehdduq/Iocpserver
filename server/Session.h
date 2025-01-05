#pragma once
#include"types.h"
#include"Over.h"

class Session 
{
public:
	// �⺻ ������
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
		memset(_name, 0, sizeof(_name)); // _name �迭 �ʱ�ȭ
	}
	void WorkerThread(Over* over, const DWORD& num_btytes);
	void DoRecv();
	void DoSend(void* packet);
	void disconnect() {
		_manuallyDisconnected = true;
		closesocket(_clientsocket); // ���� �ݱ�
	}
	bool isDisconnected() const {
		// 1. ��������� ������ ����
		if (_manuallyDisconnected) {
			return true;
		}

		// 2. ���� ���� Ȯ��
		if (_clientsocket == INVALID_SOCKET) {
			return true;
		}

		// 3. �񵿱� �۾� ���� Ȯ�� (��: Internal ���� 0�� �ƴϸ鼭 ���� �߻�)
		if (_over._over.Internal == 0) {
			// �񵿱� �۾��� ��� ����
			return false;
		}

		// 4. �߰����� ���� ���� Ȯ��
		int error = 0;
		int len = sizeof(error);
		if (getsockopt(_clientsocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error != 0) {
			return true; // ���� ���� �߻�
		}

		return false; // ���� ���°� ����
	}

public:
	int getId() { return _id; }
	short getPosX() { return _x; }
	short getPosY() { return _y; }
	int getHp() { return _hp; }
	int getExp() { return _exp; }
	int getAtt() { return _att; }
	int getMovetime() { return _movetime; }

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
	bool NpcRandomMove();
public:
	Over _over;
	SOCKET _clientsocket;
	int _prevremain = 0;
	bool _manuallyDisconnected = false;
	bool _leftright = false;
	SectionType _section;
	mutex _lock;
	bool _isNpc;
	atomic_bool _isalive = false;
public:
	unordered_set<int> _viewlist;
	mutex _viewlock;

private:
	int _id = -1;
	short _x, _y;
	int _hp;
	int _exp;
	int _att;
	int _movetime = 0;
	//char _section = -1; // 0 :  (������) 1 : (���ʾƷ�) 2: (������ ��) 3: (������ �Ʒ�) 
	char _name[20];
};

class SessionManager {
public:
	// Singleton �������� ���� ����
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}

	void CreateSession(int id, SOCKET socket);
	void CreateNpc();
	//SessionRef CreateSession(SOCKET socket);
	//Session GetSession(Session* session);

	void AddClient(int id);
	void RemoveClient(int id);
	void Dorecv(int id);
	void WorkerThread(int id, Over* over, const DWORD& numbytes);

private:
	// ������/�Ҹ��� ����� (�̱��� ����)
	SessionManager() = default;
	~SessionManager() = default;

	// ����/�̵� ����
	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;
public:
	pair<short, short> createRandomPos();

	array<Session, MAX_CLIENT + MAX_NPC> _clients; // 
	//array<Session, MAX_NPC> _npcs;
private:
	//vector<SessionRef> _clients; // ��ü Ŭ���̾�Ʈ 


	mutex _mutex; // ������ ������ ����

};

