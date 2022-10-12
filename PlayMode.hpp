#include "Mode.hpp"


#include "Scene.hpp"
#include "Sound.hpp"

#include "Connection.hpp"
#include "Game.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode(Client &client);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	
	//latest game state (from server):
	Game game;

	//----- game state -----
	// local copy of scene, each client will have this 
	Scene scene;
	//camera:
	Scene::Camera *camera = nullptr;
	Scene::Transform *P1_Paddel = nullptr;
	Scene::Transform *P2_Paddel = nullptr;

	Scene::Transform *P1_Long_Paddel = nullptr;
	Scene::Transform *P2_Long_Paddel = nullptr;

	Scene::Transform* Power_Up[20];

	Scene::Transform* Balls[20];
	Scene::Transform* P1_Bricks[84];
	Scene::Transform* P2_Bricks[84];
	//input tracking for local player:
	Player::Controls controls;


	// Music Controls 
	std::shared_ptr< Sound::PlayingSample > MainLoop;
	

	//last message from server:
	std::string server_message;
	int firstime_flg = 1;
	//connection to server:
	Client &client;

};
