#pragma once

#include <glm/glm.hpp>
//#include "PlayMode.hpp"
#include <string>
#include <list>
#include <random>
#include "Scene.hpp"
struct Connection;


//Game state, separate from rendering.

//Currently set up for a "client sends controls" / "server sends whole state" situation.

enum class Message : uint8_t {
	C2S_Controls = 1, //Greg!
	S2C_State = 's',
	//...
};

//used to represent a control input:
struct Button {
	uint8_t downs = 0; //times the button has been pressed
	bool pressed = false; //is the button pressed now
};

//state of one player in the game:
struct Player {
	//player inputs (sent from client):
	struct Controls {
		Button left, right, up, down, space;

		void send_controls_message(Connection *connection) const;

		//returns 'false' if no message or not a controls message,
		//returns 'true' if read a controls message,
		//throws on malformed controls message
		bool recv_controls_message(Connection *connection);
	} controls;

	//player state (sent from server):
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	std::string name = "";
};

struct Game {
	std::list< Player > players; //(using list so they can have stable addresses)
	Player *spawn_player(); //add player the end of the players list (may also, e.g., play some spawn anim)
	void remove_player(Player *); //remove player from game (may also, e.g., play some despawn anim)
	Player *assign_player();
	std::mt19937 mt; //used for spawning players
	uint32_t next_player_number = 1; //used for naming players

	Game();
	
	// Players
	Player Object[212];
	int P1_assigned = 0, P2_assigned = 0;
	
	// Object 0 and 1 P1 P2
	// Object 2-21 Balls
	// Object 22 - 105 P1 Bricks
	// Object 106 - 189 P2 Bricks 
	// Object 190 - P1 Long
	// Object 191 - P2 Long
	// Object 192 - 211 - Power Ups
	glm::vec2 Object_Ori_Pos[212];
	
	glm::vec2 paddle_dim;
	glm::vec2 paddle_dim_P1;
	glm::vec2 paddle_dim_P2;
	glm::vec2 brick_dim;
	glm::vec2 ball_dim;
	glm::vec2 powerup_dim;
	glm::vec2 longpaddle_dim;
	glm::vec2 horizontal_bounds;
	glm::vec2 vertical_bounds;

	// Helper finctions
	void init(int state);
	void hide_object(Player *obj);
	bool check_overlap(Player *obj1, glm::vec2 size1, Player *obj2, glm::vec2 size2);
	void swap_ball(Player *obj, int p);
	// Game Control 
	int P1_score, P2_score;
	uint8_t begin_game;
	float P1_Speed, P2_Speed;
	glm::vec2 P1_Ball_Spawn_Loc, P2_Ball_Spawn_Loc;
	float ball_speed;
	float powerup_speed;
	int randnum;
	int all_brick_dir[20];
	int play_area_balls;

	uint8_t P1_powerup_bitmap[5]; // 0 - Long, 1 - Multi, 2 - Reverse Dir All, 3 - Make color, 4 - Faster Speed 
	uint8_t P2_powerup_bitmap[5];
	uint8_t P1_long, P2_long;
	uint8_t P1_fast, P2_fast;
	//state update function:
	void update(float elapsed);

	//constants:
	//the update rate on the server:
	inline static constexpr float Tick = 1.0f / 30.0f;

	//arena size:
	inline static constexpr glm::vec2 ArenaMin = glm::vec2(-0.75f, -1.0f);
	inline static constexpr glm::vec2 ArenaMax = glm::vec2( 0.75f,  1.0f);

	//player constants:
	inline static constexpr float PlayerRadius = 0.06f;
	inline static constexpr float PlayerSpeed = 2.0f;
	inline static constexpr float PlayerAccelHalflife = 0.25f;
	
	
	//---- communication helpers ----

	//used by client:
	//set game state from data in connection buffer
	// (return true if data was read)
	bool recv_state_message(Connection *connection);

	//used by server:
	//send game state.
	//  Will move "connection_player" to the front of the front of the sent list.
	void send_state_message(Connection *connection, Player *connection_player = nullptr) const;
};
