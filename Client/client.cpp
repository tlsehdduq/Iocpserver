#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>
#include <chrono>
#include<fstream>
using namespace std;

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include"../server/packet.h"
#include"../server/types.h"
sf::TcpSocket s_socket;

constexpr auto SCREEN_WIDTH = 10;
constexpr auto SCREEN_HEIGHT = 10;

constexpr int MAX_USER = 3000;
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
std::vector<std::pair<short, short>> puddle_positions = {
//{10, 510}, {20, 520}, {30, 530}, {40, 540}, {50, 550},
//{60, 560}, {70, 570}, {80, 580}, {90, 590}, {100, 600},
//{110, 610}, {120, 620}, {130, 630}, {140, 640}, {150, 650},
//{160, 660}, {170, 670}, {180, 680}, {190, 690}, {200, 700},
//{210, 710}, {220, 720}, {230, 730}, {240, 740}, {250, 750},
//{260, 760}, {270, 770}, {280, 780}, {290, 790}, {300, 800},
//{310, 810}, {320, 820}, {330, 830}, {340, 840}, {350, 850},
//{360, 860}, {370, 870}, {380, 880}, {390, 890}, {400, 900},
//{410, 910}, {420, 920}, {430, 930}, {440, 940}, {450, 950},
//{460, 960}, {470, 970}, {480, 980}, {490, 990}, {500, 1000},
//{5, 505}, {15, 515}, {25, 525}, {35, 535}, {45, 545},
//{55, 555}, {65, 565}, {75, 575}, {85, 585}, {95, 595},
//{105, 605}, {115, 615}, {125, 625}, {135, 635}, {145, 645}
};
std::vector<std::pair<short, short>> castle_point = {
{10, 20}, {50, 100}, {200, 300}, {150, 400}, {230, 120},
	{5, 250}, {60, 50}, {125, 375}, {210, 490}, {240, 10},
	{20, 220}, {100, 150}, {180, 450}, {90, 350}, {30, 40},
	{50, 80}, {220, 410}, {150, 300}, {200, 100}, {75, 275},
	{15, 20}, {60, 30}, {190, 460}, {140, 250}, {200, 60},
	{25, 240}, {110, 360}, {180, 70}, {220, 290}, {40, 50},
	{60, 170}, {120, 390}, {210, 110}, {230, 400}, {80, 300},
	{20, 10}, {150, 250}, {190, 50}, {70, 370}, {90, 400},
	{220, 120}, {130, 430}, {100, 200}, {140, 50}, {230, 360},
	{40, 450}, {180, 30}, {120, 70}, {200, 270}, {10, 40},
	{260, 20}, {300, 100}, {450, 200}, {400, 300}, {490, 400},
	{270, 250}, {360, 50}, {425, 375}, {310, 450}, {280, 10},
	{290, 220}, {350, 150}, {480, 450}, {390, 350}, {330, 40},
	{250, 80}, {470, 410}, {350, 300}, {400, 100}, {375, 275},
	{275, 20}, {460, 30}, {390, 460}, {440, 250}, {300, 60},
	{320, 240}, {410, 360}, {380, 70}, {420, 290}, {440, 50},
	{260, 170}, {320, 390}, {410, 110}, {450, 400}, {380, 300},
	{300, 10}, {450, 250}, {490, 50}, {370, 370}, {290, 400},
	{420, 120}, {350, 430}, {300, 200}, {440, 50}, {460, 360},
	{330, 450}, {380, 30}, {420, 70}, {450, 270}, {250, 40},
	{510, 20}, {550, 100}, {700, 200}, {650, 300}, {740, 400},
	{520, 250}, {610, 50}, {675, 375}, {560, 450}, {580, 10},
	{590, 220}, {620, 150}, {730, 450}, {680, 350}, {630, 40},
	{550, 80}, {670, 410}, {620, 300}, {690, 100}, {575, 275},
	{515, 20}, {660, 30}, {690, 460}, {640, 250}, {500, 60},
	{620, 240}, {710, 360}, {580, 70}, {620, 290}, {740, 50},
	{560, 170}, {620, 390}, {610, 110}, {750, 400}, {680, 300},
	{600, 10}, {650, 250}, {590, 50}, {570, 370}, {690, 400},
	{620, 120}, {750, 430}, {500, 200}, {640, 50}, {730, 360},
	{530, 450}, {580, 30}, {620, 70}, {750, 270}, {500, 40},
	{760, 20}, {800, 100}, {950, 200}, {850, 300}, {990, 400},
	{770, 250}, {860, 50}, {925, 375}, {810, 450}, {880, 10},
	{790, 220}, {850, 150}, {980, 450}, {890, 350}, {830, 40},
	{750, 80}, {970, 410}, {850, 300}, {900, 100}, {875, 275},
	{775, 20}, {960, 30}, {890, 460}, {940, 250}, {800, 60},
	{820, 240}, {910, 360}, {880, 70}, {920, 290}, {940, 50},
	{760, 170}, {820, 390}, {910, 110}, {950, 400}, {880, 300},
	{800, 10}, {950, 250}, {990, 50}, {970, 370}, {790, 400},
	{920, 120}, {850, 430}, {800, 200}, {940, 50}, {970, 360},
	{830, 450}, {880, 30}, {920, 70}, {950, 270}, {750, 40},
	{20, 510}, {50, 600}, {120, 720}, {180, 840}, {230, 910},
	{10, 530}, {90, 650}, {140, 750}, {200, 890}, {250, 970},
	{30, 560}, {70, 640}, {160, 760}, {190, 830}, {220, 920},
	{50, 590}, {130, 710}, {170, 800}, {210, 850}, {240, 980},
	{60, 520}, {100, 630}, {190, 770}, {200, 820}, {230, 930},
	{40, 580}, {80, 720}, {150, 810}, {180, 860}, {240, 910},
	{20, 610}, {110, 690}, {120, 730}, {140, 790}, {230, 850},
	{30, 550}, {130, 650}, {170, 830}, {210, 870}, {220, 940},
	{10, 600}, {50, 740}, {160, 820}, {180, 890}, {250, 960},
	{270, 510}, {300, 620}, {320, 740}, {400, 820}, {490, 910},
	{280, 530}, {350, 650}, {390, 750}, {410, 890}, {470, 980},
	{300, 540}, {320, 660}, {360, 760}, {440, 850}, {480, 920},
	{260, 510}, {370, 610}, {420, 790}, {430, 830}, {490, 910},
	{310, 520}, {340, 630}, {400, 750}, {420, 820}, {470, 890},
	{290, 590}, {370, 720}, {380, 810}, {440, 860}, {480, 910},
	{270, 650}, {310, 710}, {330, 770}, {360, 830}, {490, 860},
	{300, 550}, {380, 640}, {400, 730}, {410, 810}, {460, 940},
	{510, 510}, {600, 600}, {700, 720}, {650, 850}, {740, 900},
	{520, 530}, {610, 670}, {675, 780}, {560, 840}, {690, 930},
	{730, 590}, {620, 640}, {710, 780}, {660, 890}, {600, 920},
	{550, 510}, {670, 690}, {690, 790}, {680, 870}, {590, 910},
	{610, 520}, {700, 610}, {600, 750}, {710, 830}, {740, 880},
	{510, 600}, {610, 720}, {700, 830}, {720, 890}, {690, 910},
	{600, 650}, {640, 750}, {660, 820}, {700, 850}, {730, 860},
	{510, 550}, {590, 640}, {620, 770}, {700, 810}, {720, 940},
	{760, 510}, {800, 620}, {850, 740}, {950, 820}, {900, 910},
	{770, 530}, {860, 650}, {875, 780}, {960, 890}, {880, 980},
	{800, 590}, {850, 640}, {930, 750}, {880, 850}, {970, 920},
	{760, 510}, {860, 710}, {900, 780}, {920, 830}, {970, 910},
	{800, 520}, {850, 630}, {890, 750}, {910, 820}, {960, 890},
	{780, 590}, {860, 720}, {900, 810}, {920, 860}, {980, 910},
	{750, 650}, {810, 720}, {830, 770}, {880, 830}, {930, 860},
	{780, 550}, {860, 640}, {910, 730}, {920, 810}, {950, 940}

};
std::vector<std::pair<short, short>> tree_positions = {
//{705, 705}, {715, 715}, {725, 725}, {735, 735}, {745, 745},
//{755, 755}, {765, 765}, {775, 775}, {785, 785}, {795, 795},
//{805, 805}, {815, 815}, {825, 825}, {835, 835}, {845, 845},
//{855, 855}, {865, 865}, {875, 875}, {885, 885}, {895, 895},
//{905, 905}, {915, 915}, {925, 925}, {935, 935}, {945, 945},
//{955, 955}, {965, 965}, {975, 975}, {985, 985}, {995, 995},
//{510, 10}, {520, 20}, {530, 30}, {540, 40}, {550, 50},
//{560, 60}, {570, 70}, {580, 80}, {590, 90}, {600, 100},
//{610, 110}, {620, 120}, {630, 130}, {640, 140}, {650, 150},
//{660, 160}, {670, 170}, {680, 180}, {690, 190}, {700, 200},
//{710, 210}, {720, 220}, {730, 230}, {740, 240}, {750, 250},
//{760, 260}, {770, 270}, {780, 280}, {790, 290}, {800, 300},
//{810, 310}, {820, 320}, {830, 330}, {840, 340}, {850, 350},
//{860, 360}, {870, 370}, {880, 380}, {890, 390}, {900, 400},
//{910, 410}, {920, 420}, {930, 430}, {940, 440}, {950, 450},
//{960, 460}, {970, 470}, {980, 480}, {990, 490}, {1000, 500},
//{505, 5}, {515, 15}, {525, 25}, {535, 35}, {545, 45},
//{555, 55}, {565, 65}, {575, 75}, {585, 85}, {595, 95},
//{605, 105}, {615, 115}, {625, 125}, {635, 135}, {645, 145},
//{655, 155}, {665, 165}, {675, 175}, {685, 185}, {695, 195},
//{705, 205}, {715, 215}, {725, 225}, {735, 235}, {745, 245},
//{755, 255}, {765, 265}, {775, 275}, {785, 285}, {795, 295},
//{805, 305}, {815, 315}, {825, 325}, {835, 335}, {845, 345},
//{855, 355}, {865, 365}, {875, 375}, {885, 385}, {895, 395},
//{905, 405}, {915, 415}, {925, 425}, {935, 435}, {945, 445},
//{955, 455}, {965, 465}, {975, 475}, {985, 485}, {995, 495}
};

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


	////////////////////// wc //////////////////////////
	ChatUI->loadFromFile("chat.png");
	/// ////////////////////////////////////////////////

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

	avatar = OBJECT{ *player,1, 0, 900, 900 };
	avatar.set_scale(0.1, 0.1);
	avatar.move(4, 4);

	////////////////////// wc //////////////////////////
	chatUI = OBJECT{ *ChatUI, 0, 0, 400, 206 };
	chatUI.a_move(500, 500);
	/// ////////////////////////////////////////////////
	//cout << puddle_positions.size() << endl;
	//cout << castle_point.size() << endl;
	//cout << tree_positions.size() << endl;

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
	for (auto& pl : players) pl.second.draw();
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
	Monstercnttext.move(600, 0);

	g_window->draw(text);
	g_window->draw(Monstercnttext);
	////////////////////// wc //////////////////////////
	chatUI.a_move(0, 450);
	chatUI.a_draw();

	chatmessage.setPosition(60, 620);

	int chatSize = curChatMessage.size();

	for (int i = 0; i < chatSize; ++i) {
		curChatMessage[i].setPosition(20, 590 - i * 30);

		g_window->draw(curChatMessage[i]);
	}
	g_window->draw(chatmessage);
	/// ////////////////////////////////////////////////


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
void Skill(int type) {

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
					// Skill 1 
				}
				case sf::Keyboard::W:
				{
					// Skill 2 
				}

				default:
					if (g_isChat) {
						info += char(event.key.code) + 97;

						chatmessage.setFont(g_font);
						chatmessage.setString(info);
						chatmessage.setFillColor(sf::Color(255, 255, 255));
					}
					///////////////////////////////////////////////////
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
					if (pos.x > 60 && pos.x < 350 && pos.y > 620 && pos.y < 640)
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

////////////////////// wc //////////////////////////
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
	curChatMessage[0].setStyle(sf::Text::Bold);
}

///////////////////////////////////////////////////
