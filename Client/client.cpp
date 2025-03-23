#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>
#include <chrono>
#include<fstream>
#include<queue>
using namespace std;

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include"../server/packet.h"
#include"../server/types.h"
sf::TcpSocket s_socket;

constexpr auto SCREEN_WIDTH = 12;
constexpr auto SCREEN_HEIGHT = 12;


constexpr int MAX_TROLL = 100000;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;

int g_left_x;
int g_top_y;
int g_myid;
char Name[NAME_SIZE];

////////////////////// wc //////////////////////////
bool g_isChat;
sf::Text chatmessage;
vector<sf::Text> curChatMessage;
vector<pair<int, int>> obstacles;

void CreateChatMessage(string _message);
void SetCurMessage(string _message);
/// ////////////////////////////////////////////////

sf::RenderWindow* g_window;
sf::Font g_font;

class OBJECT {
public:
	bool m_isleft = false;
	bool m_isattack = false;
	bool m_showing;
	sf::Sprite m_sprite;

	sf::Text m_name;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;

	int m_currentFrame;
	int m_frameCount;
	sf::Vector2i m_frameSize;
	sf::Clock m_animationClock;
	float m_animationSpeed;

	sf::Clock m_displayTimer;   // 타이머
	float m_displayDuration = 0.7f;  // 몇 초 동안 표시할지 설정 (기본 1초)

public:
	int id;
	int m_x, m_y;
	char name[NAME_SIZE];
	bool mleft = false;
	int monstercnt = 0;
	int m_level = 0;
	int m_hp = 100;
	vector<OBJECT> avatarSkill;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}
	OBJECT() {
		m_showing = false;
	}
	OBJECT(const OBJECT& other)
	{
		// 기본 멤버 변수 복사
		m_isleft = other.m_isleft;
		m_isattack = other.m_isattack;
		m_showing = other.m_showing;
		m_sprite = other.m_sprite;
		m_name = other.m_name;
		m_chat = other.m_chat;
		m_mess_end_time = other.m_mess_end_time;
		m_currentFrame = other.m_currentFrame;
		m_frameCount = other.m_frameCount;
		m_frameSize = other.m_frameSize;
		m_animationClock = other.m_animationClock;
		m_animationSpeed = other.m_animationSpeed;
		m_displayTimer = other.m_displayTimer;
		m_displayDuration = other.m_displayDuration;

		id = other.id;
		m_x = other.m_x;
		m_y = other.m_y;

		mleft = other.mleft;
		monstercnt = other.monstercnt;
		m_level = other.m_level;
		m_hp = other.m_hp;
	}

	void updateAnimation() {
		if (m_animationClock.getElapsedTime().asSeconds() > m_animationSpeed) {
			m_currentFrame = (m_currentFrame + 1) % m_frameCount;
			m_sprite.setTextureRect(sf::IntRect(m_frameSize.x * m_currentFrame, 0, m_frameSize.x, m_frameSize.y));
			m_animationClock.restart();
		}
	}
	void attackAnimation() {

	}

	void update() {
		if (m_showing && m_displayTimer.getElapsedTime().asSeconds() > m_displayDuration) {
			hide();  // 일정 시간이 지나면 숨김
		}
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		//updateAnimation();
		float rx = (m_x - g_left_x) * 65.0f + 1;
		float ry = (m_y - g_top_y) * 65.0f + 1;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx + 32 - size.width / 2, ry - 10);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx + 32 - size.width / 2, ry - 10);
			g_window->draw(m_chat);
		}
	}
	void set_scale(float x, float y) {
		m_sprite.setScale(x, y);
	}
	void set_name(const char str[]) {
		if (id < MAX_USER)
			m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(0, 0, 0));
		m_name.setStyle(sf::Text::Bold);
	}

	void set_chat(const char str[]) {
		m_chat.setFont(g_font);
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
};

OBJECT avatar;



unordered_map <int, OBJECT> players;
unordered_map <int, OBJECT> monsters;
OBJECT white_tile;
OBJECT black_tile;
OBJECT Monser;
OBJECT MonsterAttack;


OBJECT puddle;
OBJECT castle;
OBJECT Tree;

////////////////////// wc //////////////////////////
OBJECT chatUI;
sf::Texture* ChatUI;
/// ////////////////////////////////////////////////
sf::Texture* player;
sf::Texture* playerAtt;
sf::Texture* playerL;
sf::Texture* playerLAtt;
sf::Texture* playerSkill;

sf::Texture* board;
sf::Texture* obstacle;
sf::Texture* castle_obstacle;
sf::Texture* Tree_obs;
sf::Texture* devil;
sf::Texture* Dragon;
sf::Texture* MonsterAtt;



void client_initialize()
{
	board = new sf::Texture;
	obstacle = new sf::Texture;
	castle_obstacle = new sf::Texture;
	Tree_obs = new sf::Texture;
	devil = new sf::Texture;
	Dragon = new sf::Texture;
	player = new sf::Texture;
	playerAtt = new sf::Texture;
	playerL = new sf::Texture;
	playerLAtt = new sf::Texture;
	MonsterAtt = new sf::Texture;
	playerSkill = new sf::Texture;
	////////////////////// wc //////////////////////////
	ChatUI = new sf::Texture;
	/// ////////////////////////////////////////////////asd

	board->loadFromFile("MapTile.png");
	obstacle->loadFromFile("block.png");
	castle_obstacle->loadFromFile("block.png");
	Tree_obs->loadFromFile("tree.png");
	player->loadFromFile("Reaper.png");
	devil->loadFromFile("Troll.png");
	Dragon->loadFromFile("Dragon.png");
	playerAtt->loadFromFile("ReaperAtt.png");
	playerL->loadFromFile("Reaper_left.png");
	playerLAtt->loadFromFile("ReaperAtt_left.png");
	MonsterAtt->loadFromFile("MonsterAtt.png");
	playerSkill->loadFromFile("skill.png");
	ChatUI->loadFromFile("chat.png");

	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	puddle = OBJECT{ *obstacle,0,0,TILE_WIDTH,TILE_WIDTH };
	castle = OBJECT{ *castle_obstacle,0,0,TILE_WIDTH,TILE_WIDTH };
	Tree = OBJECT{ *Tree_obs,0,0,32,32 };
	Tree.set_scale(2.0, 2.0);

	avatar = OBJECT{ *player,1,0, 900, 900 };
	avatar.set_scale(0.1, 0.1);
	avatar.move(4, 4);

	chatUI = OBJECT{ *ChatUI,0, 0, 400, 206 };
	chatUI.a_move(500, 500);

}

void client_finish()
{
	players.clear();
	delete board;
	delete player;

	////////////////////// wc //////////////////////////
	delete ChatUI;
	/// ////////////////////////////////////////////////
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN:
	{
		SC_LOGIN_PACKET* packet = reinterpret_cast<SC_LOGIN_PACKET*>(ptr);
		g_myid = packet->id;
		avatar.id = g_myid;
		avatar.move(packet->x, packet->y);
		g_left_x = packet->x - SCREEN_WIDTH / 2;
		g_top_y = packet->y - SCREEN_HEIGHT / 2;
		avatar.show();
	}
	break;

	case SC_ADD_OBJECT:
	{
		SC_ADD_PACKET* my_packet = reinterpret_cast<SC_ADD_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			g_top_y = my_packet->y - SCREEN_HEIGHT / 2;
			avatar.show();
		}
		else if (id < MAX_USER) {
			////////////////////// wc //////////////////////////
			//players[id] = OBJECT{ *pieces, 64, 0, 64, 64 };
			players[id] = OBJECT{ *player, 0, 0, 900, 900 };
			players[id].set_scale(0.1, 0.1);
			///////////////////////////////////////////////////

			players[id].id = id;
			players[id].move(my_packet->x, my_packet->y);
			players[id].set_name(my_packet->name);
			players[id].show();
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			g_top_y = my_packet->y - SCREEN_HEIGHT / 2;
		}

		else {
			if (my_packet->left && !players[other_id].m_isleft) {
				players[other_id].m_isleft = true;
				auto previousPosition = players[other_id].m_sprite.getPosition();
				auto currentScale = players[other_id].m_sprite.getScale();
				players[other_id].m_sprite.setTexture(*playerL);
				players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
				players[other_id].m_sprite.setPosition(previousPosition);
				players[other_id].m_sprite.setScale(currentScale);
			}
			else if (my_packet->left == false && players[other_id].m_isleft)
			{

				players[other_id].m_isleft = false;
				auto previousPosition = players[other_id].m_sprite.getPosition();
				auto currentScale = players[other_id].m_sprite.getScale();
				players[other_id].m_sprite.setTexture(*player);
				players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
				players[other_id].m_sprite.setPosition(previousPosition);
				players[other_id].m_sprite.setScale(currentScale);

			}
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}
	case SC_PLAYER_ATTACK: {
		SC_PLAYER_ATTACK_PACKET* p = reinterpret_cast<SC_PLAYER_ATTACK_PACKET*>(ptr);
		int other_id = p->id;
		if (p->onoff) {
			if (p->id != g_myid && players[other_id].m_isattack == false) {
				players[other_id].m_isattack = p->onoff;
				if (players[other_id].m_isleft) {
					auto previousPosition = players[other_id].m_sprite.getPosition();
					auto currentScale = players[other_id].m_sprite.getScale();
					players[other_id].m_sprite.setTexture(*playerLAtt);
					players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					players[other_id].m_sprite.setPosition(previousPosition);
					players[other_id].m_sprite.setScale(currentScale);
				}
				else
				{
					auto previousPosition = players[other_id].m_sprite.getPosition();
					auto currentScale = players[other_id].m_sprite.getScale();
					players[other_id].m_sprite.setTexture(*playerAtt);
					players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					players[other_id].m_sprite.setPosition(previousPosition);
					players[other_id].m_sprite.setScale(currentScale);
				}
			}
		}
		else
		{
			if (p->id != g_myid && players[other_id].m_isattack == true) {
				players[other_id].m_isattack = p->onoff;
				if (players[other_id].m_isleft) {
					auto previousPosition = players[other_id].m_sprite.getPosition();
					auto currentScale = players[other_id].m_sprite.getScale();
					players[other_id].m_sprite.setTexture(*playerL);
					players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					players[other_id].m_sprite.setPosition(previousPosition);
					players[other_id].m_sprite.setScale(currentScale);
				}
				else
				{
					auto previousPosition = players[other_id].m_sprite.getPosition();
					auto currentScale = players[other_id].m_sprite.getScale();
					players[other_id].m_sprite.setTexture(*player);
					players[other_id].m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					players[other_id].m_sprite.setPosition(previousPosition);

				}
			}
		}
	}
						 break;
	case SC_PLAYER_LEVELUP:
	{
		SC_PlAYER_LEVEL_UP_PACKET* p = reinterpret_cast<SC_PlAYER_LEVEL_UP_PACKET*>(ptr);
		avatar.m_level = p->level;
	}
	break;
	case SC_PLAYER_SKILL:
	{
		SC_PLAYER_SKILL_PACKET* p = reinterpret_cast<SC_PLAYER_SKILL_PACKET*>(ptr);
		int c_id = p->id;
		if (p->id == avatar.id)
		{
			if (!avatar.avatarSkill.empty()) avatar.avatarSkill.clear();
		}
		else
		{
			if (!players[c_id].avatarSkill.empty())players[c_id].avatarSkill.clear();
		}
		short directions[8][2] = {
			{-1, -1}, {0, -1}, {1, -1},
			{-1, 0},           {1, 0},
			{-1, 1},  {0, 1},  {1, 1}
		};
		pair<short, short> pos = { p->x,p->y };
		vector<pair<short, short>> skillpos;
		for (const auto& dir : directions)
		{
			skillpos.push_back({ (pos.first + dir[0]), (pos.second + dir[1]) });
		}
		for (const auto& s_p : skillpos)
		{
			OBJECT Temp{ *playerSkill,0,0,1530,1257 };
			Temp.move(s_p.first, s_p.second);
			Temp.set_scale(0.07, 0.07);
			Temp.set_name(" ");
			Temp.show();
			if (c_id == avatar.id)
				avatar.avatarSkill.push_back(Temp);
			else
				players[c_id].avatarSkill.push_back(Temp);
		}

	}
	break;
	case SC_REMOVE:
	{
		SC_REMOVE_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (my_packet->sessiontype == 1) {

			if (other_id == g_myid) {
				avatar.hide();
			}
			else {
				players.erase(other_id);
			}
		}
		else {
			avatar.monstercnt++;
			monsters[other_id].hide();
			monsters.erase(other_id);
		}
		break;
	}
	case SC_MONSTER_INIT:
	{
		SC_MONTSER_INIT_PACKET* p = reinterpret_cast<SC_MONTSER_INIT_PACKET*>(ptr);
		int npc_id = p->id;
		if (npc_id < MAX_TROLL)
		{
			monsters[npc_id] = OBJECT{ *devil,0,0,72,108 };
			monsters[npc_id].m_sprite.setScale(1.0, 1.0);
			monsters[npc_id].id = p->id;
			monsters[npc_id].move(p->x, p->y);
			monsters[npc_id].set_name("Troll");
		}
		else
		{
			monsters[npc_id] = OBJECT{ *Dragon,0,0,256,256 };
			monsters[npc_id].m_sprite.setScale(1.0, 1.0);
			monsters[npc_id].id = p->id;
			monsters[npc_id].move(p->x, p->y);
			monsters[npc_id].set_name("Dragon");
		}
		monsters[npc_id].show();
	}
	break;
	case SC_MONSTER_MOVE:
	{
		SC_MONSTER_MOVE_PACKET* p = reinterpret_cast<SC_MONSTER_MOVE_PACKET*>(ptr);
		int npc_id = p->id;
		monsters[npc_id].m_x = p->x;
		monsters[npc_id].m_y = p->y;

	}
	break;
	case SC_MONSTER_REMOVE:
	{
		SC_MONSTER_REMOVE_PACKET* p = reinterpret_cast<SC_MONSTER_REMOVE_PACKET*>(ptr);
		int npc_id = p->id;
		monsters.erase(npc_id);


	}
	break;
	case SC_CHAT:
	{
		SC_CHAT_PACKET* p = reinterpret_cast<SC_CHAT_PACKET*>(ptr);

		string info = "[";
		if (avatar.id == p->id)
			info += to_string(avatar.id);
		else
			info += to_string(p->id);
		info += "]:";
		info += p->message;
		CreateChatMessage(info);

		break;
	}
	case SC_MONSTER_ATTACK:
	{
		SC_MONSTER_ATTACK_PACKET* p = reinterpret_cast<SC_MONSTER_ATTACK_PACKET*>(ptr);
		int npc_id = p->id;
		int x = monsters[npc_id].m_x;
		int y = monsters[npc_id].m_y;

		string info = "[";
		info += to_string(p->id);
		info += " NPC ATTACK TO ";
		info += to_string(p->target_id);
		info += " PLAYER ]";

		CreateChatMessage(info);
		MonsterAttack = OBJECT{ *MonsterAtt,0,0,72,72 };
		MonsterAttack.m_sprite.setScale(1.5, 1.5);
		MonsterAttack.move(x, y);
		MonsterAttack.set_name("ATTACK");
		MonsterAttack.show();
		MonsterAttack.m_displayTimer.restart();
	}break;
	///////////////////////////////////////////////////
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = s_socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		exit(-1);
	}
	if (recv_result == sf::Socket::Disconnected) {
		wcout << L"Disconnected\n";
		exit(-1);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;

			bool is_castle = false;
			for (const auto& c_puddle : obstacles) {
				if (tile_x == c_puddle.first && tile_y == c_puddle.second) {
					is_castle = true;
					break;
				}
			}
			if (is_castle) {
				// 장애물 렌더링
				castle.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
				castle.a_draw();
			}
			else {
				//기본 타일 렌더링
				if (0 == (tile_x / 3 + tile_y / 3) % 2) {
					white_tile.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
					white_tile.a_draw();
				}
				else {
					black_tile.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
					black_tile.a_draw();
				}
			}
		}
	avatar.draw();
	for (auto& skill : avatar.avatarSkill)
	{
		skill.update();
		skill.draw();
	}
	for (auto& pl : players)
	{
		pl.second.draw();
		for (auto& skill : pl.second.avatarSkill)
		{
			skill.update();
			skill.draw();
		}
	}
	for (auto& monster : monsters) {
		monster.second.draw();
	}

	if (MonsterAttack.m_showing) {
		MonsterAttack.update();
		MonsterAttack.draw();
	}


	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);

	sf::Text Monstercnttext;
	Monstercnttext.setFont(g_font);
	Monstercnttext.setFillColor(sf::Color(255, 100, 0));
	Monstercnttext.setStyle(sf::Text::Bold);

	char tbuf[10];
	sprintf_s(tbuf, "%d", avatar.monstercnt);
	Monstercnttext.setString(tbuf);
	Monstercnttext.move(700, 0);

	sf::Text PlayerLevel;
	PlayerLevel.setFont(g_font);
	PlayerLevel.setFillColor(sf::Color(100, 100, 200));
	PlayerLevel.setStyle(sf::Text::Bold);

	char tbuf2[10];
	sprintf_s(tbuf2, "[LV. %d]", avatar.m_level);
	PlayerLevel.setString(tbuf2);
	PlayerLevel.move(0, 30);

	g_window->draw(text);
	g_window->draw(Monstercnttext);
	g_window->draw(PlayerLevel);

	chatUI.a_move(0, 575);
	chatUI.a_draw();

	chatmessage.setPosition(60, 740);

	int chatSize = curChatMessage.size();

	for (int i = 0; i < chatSize; ++i) {
		curChatMessage[i].setPosition(15, 700 - i * 30);

		g_window->draw(curChatMessage[i]);
	}
	g_window->draw(chatmessage);

}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	s_socket.send(packet, p[0], sent);
}
void Login()
{
	cout << "Name : ";
	cin >> Name;
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;

	avatar.set_name(Name);

	strcpy_s(p.name, Name);
	send_packet(&p);
}
void Attack() {
	CS_ATTACK_PACKET p;
	p.size = sizeof(CS_ATTACK_PACKET);
	p.type = CS_ATTACK;
	send_packet(&p);
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = s_socket.connect("127.0.0.1", 7777);
	s_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		exit(-1);
	}
	std::ifstream inFile("obstacles.txt");
	if (!inFile)
	{
		cout << " Failed obstacles load " << endl;
	}

	int x, y;

	while (inFile >> x >> y)
	{
		obstacles.emplace_back(x, y);
	}
	cout << " Obstacle Settting Complete" << endl;

	client_initialize();

	Login();

	sf::Vector2i pos;
	string cchat;
	string info;

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FINAL PROJECT");
	g_window = &window;

	sf::Vector2f previousPosition;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int direction = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left: {

					direction = 2;
					previousPosition = avatar.m_sprite.getPosition();
					auto currentScale = avatar.m_sprite.getScale();

					// 텍스처만 공격 모션 텍스처로 변경
					avatar.m_sprite.setTexture(*playerL);
					avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					avatar.mleft = true;
					// 기존 위치와 크기 복원
					avatar.m_sprite.setPosition(previousPosition);
					avatar.m_sprite.setScale(currentScale);
				}
									   break;
				case sf::Keyboard::Right: {

					direction = 3;
					previousPosition = avatar.m_sprite.getPosition();
					auto currentScale = avatar.m_sprite.getScale();

					// 텍스처만 공격 모션 텍스처로 변경
					avatar.m_sprite.setTexture(*player);
					avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					avatar.mleft = false;
					// 기존 위치와 크기 복원
					avatar.m_sprite.setPosition(previousPosition);
					avatar.m_sprite.setScale(currentScale);
				}
										break;
				case sf::Keyboard::Up:
					direction = 0;
					break;
				case sf::Keyboard::Down:
					direction = 1;
					break;
				case sf::Keyboard::Escape:
					// 로그아웃 패킷 
					CS_LOGOUT_PACKET logoutpacket;
					logoutpacket.size = sizeof(CS_LOGOUT_PACKET);
					logoutpacket.type = CS_LOGOUT;
					send_packet(&logoutpacket);

					// 소켓 연결 해제
					s_socket.disconnect();

					// 창 닫기
					window.close();
					return 0;

					break;
				case sf::Keyboard::Enter:
					CS_CHAT_PACKET chat_packet;
					chat_packet.size = sizeof(chat_packet) - sizeof(chat_packet.message) + strlen(info.c_str()) + 1;
					chat_packet.type = CS_CHAT;

					strcpy_s(chat_packet.message, info.c_str());
					send_packet(&chat_packet); // 메시지를 보냄 
					info = "";
					g_isChat = false;
					chatmessage.setString(info);
					g_window->draw(chatmessage);
					break;
				case sf::Keyboard::Space:
				{
					previousPosition = avatar.m_sprite.getPosition();
					auto currentScale = avatar.m_sprite.getScale();

					if (avatar.mleft) {
						avatar.m_sprite.setTexture(*playerLAtt);
						avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					}
					else {
						// 텍스처만 공격 모션 텍스처로 변경
						avatar.m_sprite.setTexture(*playerAtt);
						avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					}
					// 기존 위치와 크기 복원
					avatar.m_sprite.setPosition(previousPosition);
					avatar.m_sprite.setScale(currentScale);

					Attack();

					break;
				}
				case sf::Keyboard::Q:
				{
					//if (avatar.m_level <= 3)break;
					CS_PLAYER_SKILL_PACKET packet;
					packet.size = sizeof(CS_PLAYER_SKILL_PACKET);
					packet.type = CS_PLAYER_SKILL;

					send_packet(&packet);
					break;
				}
				case sf::Keyboard::W:
				{

				}

				default:
					if (g_isChat) {
						info += char(event.key.code) + 97;

						chatmessage.setFont(g_font);
						chatmessage.setString(info);
						chatmessage.setFillColor(sf::Color(255, 255, 255));
					}
					break;
				}
				if (-1 != direction) {
					CS_MOVE_PLAYER_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE_PLAYER;
					p.dir = direction;
					send_packet(&p);
				}

			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Space) {
					// 스페이스바가 올라왔을 때 원래 모션으로 복귀
					previousPosition = avatar.m_sprite.getPosition();
					auto currentScale = avatar.m_sprite.getScale();
					if (avatar.mleft) {
						avatar.m_sprite.setTexture(*playerL);
						avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					}
					else {
						avatar.m_sprite.setTexture(*player);
						avatar.m_sprite.setTextureRect(sf::IntRect(0, 0, 900, 900));
					}

					avatar.m_sprite.setPosition(previousPosition);
					avatar.m_sprite.setScale(currentScale);
				}
			}
			////////////////////// wc //////////////////////////		
			if (event.type == sf::Event::MouseButtonPressed)
			{
				switch (event.key.code)
				{
				case sf::Mouse::Left:
					pos = sf::Mouse::getPosition(window);
					cout << pos.x << " , " << pos.y << endl;
					if (pos.x > 60 && pos.x < 350 && pos.y > 750 && pos.y < 770)
						g_isChat = !g_isChat;
					break;
				default:
					break;
				}
			}
			///////////////////////////////////////////////////
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}

void CreateChatMessage(string _message)
{
	int chatSize = curChatMessage.size();

	if (curChatMessage.size() < 5)
		curChatMessage.push_back(sf::Text());

	if (curChatMessage.size() == 1) {
		SetCurMessage(_message);
	}
	else if (curChatMessage.size() == 2) {
		curChatMessage[1] = curChatMessage[0];
		SetCurMessage(_message);
	}
	else if (curChatMessage.size() == 3) {
		curChatMessage[2] = curChatMessage[1];
		curChatMessage[1] = curChatMessage[0];
		SetCurMessage(_message);
	}
	else if (curChatMessage.size() == 4) {
		curChatMessage[3] = curChatMessage[2];
		curChatMessage[2] = curChatMessage[1];
		curChatMessage[1] = curChatMessage[0];
		SetCurMessage(_message);
	}
	else if (curChatMessage.size() == 5) {
		curChatMessage[4] = curChatMessage[3];
		curChatMessage[3] = curChatMessage[2];
		curChatMessage[2] = curChatMessage[1];
		curChatMessage[1] = curChatMessage[0];
		SetCurMessage(_message);
	}
}

void SetCurMessage(string _message)
{
	curChatMessage[0].setFont(g_font);
	curChatMessage[0].setString(_message);
	curChatMessage[0].setFillColor(sf::Color(255, 255, 255));
	curChatMessage[0].setScale(0.6, 0.6);
	curChatMessage[0].setStyle(sf::Text::Bold);
}

