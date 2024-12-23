#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>
#include <chrono>
using namespace std;

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include"../Server/protocol.h"
sf::TcpSocket s_socket;

constexpr auto SCREEN_WIDTH = 10;
constexpr auto SCREEN_HEIGHT = 10;

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

void CreateChatMessage(string _message);
void SetCurMessage(string _message);
/// ////////////////////////////////////////////////

sf::RenderWindow* g_window;
sf::Font g_font;
std::vector<std::pair<short, short>> puddle_positions = {
	{10, 15}, {50, 100}, {90, 120}, {150, 200}, {200, 250},
	{30, 40}, {80, 90}, {130, 140}, {180, 190}, {240, 250},
	{70, 20}, {120, 30}, {170, 40}, {220, 50}, {270, 60},
	{300, 300}, {320, 350}, {400, 400}, {450, 450}, {480, 490},
	{20, 300}, {60, 320}, {100, 350}, {200, 400}, {300, 450},
	{50, 250}, {100, 260}, {150, 270}, {200, 280}, {250, 290},
	{400, 10}, {420, 60}, {440, 120}, {460, 180}, {480, 250},
	{50, 30}, {100, 70}, {150, 110}, {200, 150}, {250, 190},
	{300, 20}, {320, 70}, {340, 120}, {360, 170}, {380, 220},
	{400, 300}, {420, 350}, {440, 400}, {460, 450}, {490, 480},
	{30, 480}, {70, 440}, {110, 400}, {150, 360}, {190, 320},
	{210, 210}, {250, 250}, {290, 290}, {330, 330}, {370, 370},
	{15, 15}, {60, 60}, {105, 105}, {150, 150}, {195, 195},
	{240, 240}, {285, 285}, {330, 330}, {375, 375}, {420, 420},
	{460, 460}, {20, 450}, {40, 430}, {60, 410}, {80, 390},
	{250, 50}, {260, 100}, {270, 150}, {280, 200}, {290, 250},
	{300, 300}, {310, 350}, {320, 400}, {330, 450}, {340, 490},
	{400, 30}, {450, 50}, {480, 70}, {490, 90}, {490, 130},
	{50, 20}, {100, 50}, {150, 100}, {200, 150}, {250, 200},
	{300, 250}, {350, 300}, {400, 350}, {450, 400}, {490, 450},
	{20, 100}, {50, 150}, {80, 200}, {110, 250}, {140, 300},
	{300, 20}, {350, 60}, {400, 110}, {450, 160}, {490, 210},
	{100, 490}, {150, 470}, {200, 450}, {250, 430}, {300, 410},
	{350, 390}, {400, 370}, {450, 350}, {490, 330}, {40, 470},
};
std::vector<std::pair<short, short>> castle_point = {
	{5, 25}, {15, 35}, {25, 45}, {35, 55}, {45, 65},
	{55, 75}, {65, 85}, {75, 95}, {85, 105}, {95, 115},
	{105, 125}, {115, 135}, {125, 145}, {135, 155}, {145, 165},
	{155, 175}, {165, 185}, {175, 195}, {185, 205}, {195, 215},
	{205, 225}, {215, 235}, {225, 245}, {235, 255}, {245, 265},
	{255, 275}, {265, 285}, {275, 295}, {285, 305}, {295, 315},
	{305, 325}, {315, 335}, {325, 345}, {335, 355}, {345, 365},
	{355, 375}, {365, 385}, {375, 395}, {385, 405}, {395, 415},
	{405, 425}, {415, 435}, {425, 445}, {435, 455}, {445, 465},
	{455, 475}, {465, 485}, {475, 495}, {485, 5}, {495, 15},
	{5, 495}, {15, 485}, {25, 475}, {35, 465}, {45, 455},
	{55, 445}, {65, 435}, {75, 425}, {85, 415}, {95, 405},
	{105, 395}, {115, 385}, {125, 375}, {135, 365}, {145, 355},
	{155, 345}, {165, 335}, {175, 325}, {185, 315}, {195, 305},
	{205, 295}, {215, 285}, {225, 275}, {235, 265}, {245, 255},
	{255, 245}, {265, 235}, {275, 225}, {285, 215}, {295, 205},
	{305, 195}, {315, 185}, {325, 175}, {335, 165}, {345, 155},
	{355, 145}, {365, 135}, {375, 125}, {385, 115}, {395, 105},
	{405, 95}, {415, 85}, {425, 75}, {435, 65}, {445, 55},
};
std::vector<std::pair<short, short>> tree_positions = {
	{3, 7}, {12, 18}, {21, 30}, {33, 45}, {44, 55},
	{66, 12}, {75, 24}, {85, 35}, {95, 46}, {100, 58},
	{110, 23}, {120, 37}, {130, 49}, {140, 61}, {150, 72},
	{160, 83}, {170, 94}, {180, 105}, {190, 116}, {200, 127},
	{210, 138}, {220, 149}, {230, 160}, {240, 171}, {250, 182},
	{260, 193}, {270, 204}, {280, 215}, {290, 226}, {300, 237},
	{310, 248}, {320, 259}, {330, 270}, {340, 281}, {350, 292},
	{360, 303}, {370, 314}, {380, 325}, {390, 336}, {400, 347},
	{410, 358}, {420, 369}, {430, 380}, {440, 391}, {450, 402},
	{460, 413}, {470, 424}, {480, 435}, {490, 446}, {500, 457},
	{5, 10}, {15, 20}, {25, 30}, {35, 40}, {45, 50},
	{55, 60}, {65, 70}, {75, 80}, {85, 90}, {95, 100},
	{105, 110}, {115, 120}, {125, 130}, {135, 140}, {145, 150},
	{155, 160}, {165, 170}, {175, 180}, {185, 190}, {195, 200},
	{205, 210}, {215, 220}, {225, 230}, {235, 240}, {245, 250},
	{255, 260}, {265, 270}, {275, 280}, {285, 290}, {295, 300},
	{305, 310}, {315, 320}, {325, 330}, {335, 340}, {345, 350},
	{355, 360}, {365, 370}, {375, 380}, {385, 390}, {395, 400},
	{405, 410}, {415, 420}, {425, 430}, {435, 440}, {445, 450},
	{455, 460}, {465, 470}, {475, 480}, {485, 490}, {495, 500},
	{13, 17}, {23, 27}, {33, 37}, {43, 47}, {53, 57},
	{63, 67}, {73, 77}, {83, 87}, {93, 97}, {103, 107},
	{113, 117}, {123, 127}, {133, 137}, {143, 147}, {153, 157},
	{163, 167}, {173, 177}, {183, 187}, {193, 197}, {203, 207},
	{213, 217}, {223, 227}, {233, 237}, {243, 247}, {253, 257},
	{263, 267}, {273, 277}, {283, 287}, {293, 297}, {303, 307},
	{313, 317}, {323, 327}, {333, 337}, {343, 347}, {353, 357},
	{363, 367}, {373, 377}, {383, 387}, {393, 397}, {403, 407},
	{413, 417}, {423, 427}, {433, 437}, {443, 447}, {453, 457},
	{463, 467}, {473, 477}, {483, 487}, {493, 497}, {5, 487},
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

public:
	int id;
	int m_x, m_y;
	char name[NAME_SIZE];
	bool mleft = false;
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
		m_name.setFont(g_font);
		m_name.setString(str);
		if (id < MAX_USER) m_name.setFillColor(sf::Color(255, 255, 255));
		else m_name.setFillColor(sf::Color(255, 255, 0));
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
	////////////////////// wc //////////////////////////
	ChatUI = new sf::Texture;
	/// ////////////////////////////////////////////////

	board->loadFromFile("MapTile.png");
	obstacle->loadFromFile("block.png");
	castle_obstacle->loadFromFile("castle.png");
	Tree_obs->loadFromFile("tree.png");
	player->loadFromFile("Reaper.png");
	devil->loadFromFile("Devil.png");
	Dragon->loadFromFile("Dragon.png");
	playerAtt->loadFromFile("ReaperAtt.png");
	playerL->loadFromFile("Reaper_left.png");
	playerLAtt->loadFromFile("ReaperAtt_left.png");


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
			monsters[other_id].hide();
			monsters.erase(other_id);
		}
		break;
	}
	case SC_MONTSER_INIT:
	{
		SC_MONTSER_INIT_PACKET* p = reinterpret_cast<SC_MONTSER_INIT_PACKET*>(ptr);
		int npc_id = p->id;
		monsters[npc_id] = OBJECT{ *devil,0,0,161,133 };
		monsters[npc_id].m_sprite.setScale(0.5, 0.5);
		monsters[npc_id].id = p->id;
		monsters[npc_id].move(p->x, p->y);
		monsters[npc_id].set_name("Devil");
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

			// 장애물 확인
			bool is_puddle = false;
			for (const auto& puddle : puddle_positions) {
				if (tile_x == puddle.first && tile_y == puddle.second) {
					is_puddle = true;
					break;
				}
			}
			bool is_castle = false;
			for (const auto& c_puddle : castle_point) {
				if (tile_x == c_puddle.first && tile_y == c_puddle.second) {
					is_castle = true;
					break;
				}
			}
			bool is_tree = false;
			for (const auto& trees : tree_positions) {
				if (tile_x == trees.first && tile_y == trees.second) {
					is_tree = true;
					break;
				}
			}

			if (is_puddle) {
				// 장애물 렌더링
				puddle.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
				puddle.a_draw();
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
				if (is_castle) {
					castle.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
					castle.a_draw();
				}
				if (is_tree) {
					Tree.a_move(TILE_WIDTH * i, TILE_WIDTH * j);
					Tree.a_draw();
				}
			}
		}
	avatar.draw();
	for (auto& pl : players) pl.second.draw();
	for (auto& monster : monsters) {
		monster.second.draw();
	}
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);

	g_window->draw(text);
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
					window.close();
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
