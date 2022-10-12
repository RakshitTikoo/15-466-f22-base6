#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <array>


GLuint Game5_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > Game5_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("Game5.pnct"));
	Game5_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > Game5_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("Game5.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = Game5_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = Game5_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > Main_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Compy-Jazz.opus"));
});

PlayMode::PlayMode(Client &client_) : client(client_), scene(*Game5_scene) {
	//get pointers to transforms
	// set nullptr
	for(uint32_t i = 0; i < 20; i++) {
		Balls[i] = nullptr;
	}

	for(uint32_t i = 0; i < 84; i++) {
		P1_Bricks[i] = nullptr;
		P2_Bricks[i] = nullptr;
	}

	for(uint32_t i = 0; i < 20; i++) {
		Power_Up[i] = nullptr;
	}


	for (auto &transform : scene.transforms) {
		if (transform.name == "P1_Paddel") P1_Paddel = &transform;
		else if (transform.name == "P2_Paddel") P2_Paddel = &transform;
		else if (transform.name == "P1_Longer_Paddel") P1_Long_Paddel = &transform;
		else if (transform.name == "P2_Longer_Paddel") P2_Long_Paddel = &transform;
		else {

			// Balls 
			for(uint32_t i = 0; i < 20; i++) {
				std::string ball_str = std::string("Ball.00") + std::to_string(i + 1);
				if(i >= 9)
					ball_str = std::string("Ball.0") + std::to_string(i + 1);	

				if(transform.name == ball_str) Balls[i] = &transform;
			}

			// Bricks 
			for(uint32_t i = 0; i < 84; i++) {
				std::string P1_brick_str = std::string("P1_Brick.00") + std::to_string(i + 1);
				if(i >= 9)
					P1_brick_str = std::string("P1_Brick.0") + std::to_string(i + 1);	

				if(transform.name == P1_brick_str) P1_Bricks[i] = &transform;


				std::string P2_brick_str = std::string("P2_Brick.00") + std::to_string(i + 1);
				if(i >= 9)
					P2_brick_str = std::string("P2_Brick.0") + std::to_string(i + 1);	

				if(transform.name == P2_brick_str) P2_Bricks[i] = &transform;
			}

			// Power Ups 
			for(uint32_t i = 0; i < 20; i++) {
				std::string power_up_str = std::string("Power_Up.00") + std::to_string(i + 1);
				if(i >= 9)
					power_up_str = std::string("Power_Up.0") + std::to_string(i + 1);	

				if(transform.name == power_up_str) Power_Up[i] = &transform;
			}


		}
	}
	
	if (P1_Paddel == nullptr) throw std::runtime_error("P1_Paddel not found.");
	if (P2_Paddel == nullptr) throw std::runtime_error("P2_Paddel not found.");
	if (P1_Long_Paddel == nullptr) throw std::runtime_error("P1_Long_Paddel not found.");
	if (P2_Long_Paddel == nullptr) throw std::runtime_error("P2_Long_Paddel not found.");

	for(uint32_t i = 0; i < 20; i++) {
		if(Balls[i] == nullptr)  throw std::runtime_error("balls not found.");
	}

	for(uint32_t i = 0; i < 84; i++) {
		if(P1_Bricks[i] == nullptr)  throw std::runtime_error("P1_Bricks not found.");
		if(P2_Bricks[i] == nullptr)  throw std::runtime_error("P2_Bricks not found.");
	}

	for(uint32_t i = 0; i < 20; i++) {
		if(Power_Up[i] == nullptr)  throw std::runtime_error("Power_Up not found.");
	}
	// print all positions for objects
	//for(uint32_t i = 0; i < 212; i++) {
	//	if(i%10 == 0) printf("\n");
	//	if(i == 0) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P1_Paddel->position.x, i, P1_Paddel->position.y, i, P1_Paddel->name.c_str()); // P1
	//	if(i == 1) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P2_Paddel->position.x, i, P2_Paddel->position.y, i, P2_Paddel->name.c_str()); // P2
	//	if(i >= 2 && i <= 21) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, Balls[i-2]->position.x, i, Balls[i-2]->position.y, i, Balls[i-2]->name.c_str());  // Balls
	//	if(i  >= 22 && i <= 105) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P1_Bricks[i-22]->position.x, i, P1_Bricks[i-22]->position.y, i, P1_Bricks[i-22]->name.c_str()); // P1 Brick
	//	if(i  >= 106 && i <= 189) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P2_Bricks[i-106]->position.x, i, P2_Bricks[i-106]->position.y, i, P2_Bricks[i-106]->name.c_str()); // P2 Brick
	//	if(i == 190) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P1_Long_Paddel->position.x, i, P1_Long_Paddel->position.y, i, P1_Long_Paddel->name.c_str()); // P1 long
	//	if(i == 191) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, P2_Long_Paddel->position.x, i, P2_Long_Paddel->position.y, i, P2_Long_Paddel->name.c_str()); // P2 long
	//	if(i >= 192) printf("Object[%d].position.x = %ff; Object[%d].position.y = %ff; Object[%d].name = \"%s\"; ", i, Power_Up[i-192]->position.x, i, Power_Up[i-192]->position.y, i, Power_Up[i-192]->name.c_str()); // P1 Brick 
	//}
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	// camera pos x y z = 0.000000 0.000000 350.000000 camera rot w x y z = 1.000000 0.000000 0.000000 0.000000
	camera->transform->position.x = 0.0f; camera->transform->position.y = 0.0f; camera->transform->position.z = 350.0f;
	camera->transform->rotation.w = 1.0f; camera->transform->rotation.x = 0.0f; camera->transform->rotation.y = 0.0f; camera->transform->rotation.z = 0.0f;
	
	MainLoop = Sound::loop(*Main_sample, 0.0f, 0.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.repeat) {
			//ignore repeats
		} else if (evt.key.keysym.sym == SDLK_a) {
			controls.left.downs += 1;
			controls.left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			controls.right.downs += 1;
			controls.right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			controls.up.downs += 1;
			controls.up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			controls.down.downs += 1;
			controls.down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			controls.space.downs += 1;
			controls.space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			controls.left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			controls.right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			controls.up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			controls.down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			controls.space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//queue data for sending to server:
	controls.send_controls_message(&client.connection);

	//reset button press counters:
	controls.left.downs = 0;
	controls.right.downs = 0;
	controls.up.downs = 0;
	controls.down.downs = 0;
	controls.space.downs = 0;

	//send/receive data:
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush(); //DEBUG
			bool handled_message;
			try {
				do {
					handled_message = false;
					if (game.recv_state_message(c)) handled_message = true;
				} while (handled_message);
			} catch (std::exception const &e) {
				std::cerr << "[" << c->socket << "] malformed message from server: " << e.what() << std::endl;
				//quit the game:
				throw e;
			}
		}
	}, 0.0);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);	
	
	// Update positions of all objects 
	for(uint32_t i = 0; i < 212; i++) {
		if(i == 0) {
			P1_Paddel->position.x = game.Object[i].position.x;
			P1_Paddel->position.y = game.Object[i].position.y;
		} // P1
		if(i == 1) {
			P2_Paddel->position.x = game.Object[i].position.x;
			P2_Paddel->position.y = game.Object[i].position.y;
		} // P2
		if(i >= 2 && i <= 21) {
			Balls[i - 2]->position.x = game.Object[i].position.x;
			Balls[i - 2]->position.y = game.Object[i].position.y;
		}  // Balls
		if(i >= 22 && i <= 105) {
			P1_Bricks[i - 22]->position.x = game.Object[i].position.x;
			P1_Bricks[i - 22]->position.y = game.Object[i].position.y;
		} // P1 Brick
		if(i >= 106 && i <= 189) {
			P2_Bricks[i - 106]->position.x = game.Object[i].position.x;
			P2_Bricks[i - 106]->position.y = game.Object[i].position.y;
		} // P2 Brick
		if(i == 190) {
			P1_Long_Paddel->position.x = game.Object[i].position.x;
			P1_Long_Paddel->position.y = game.Object[i].position.y;
		} // P1 Long
		if(i == 191) {
			P2_Long_Paddel->position.x = game.Object[i].position.x;
			P2_Long_Paddel->position.y = game.Object[i].position.y;
		} // P2 Long
		if(i >= 192) {
			Power_Up[i-192]->position.x = game.Object[i].position.x;
			Power_Up[i-192]->position.y = game.Object[i].position.y;
		} // PowerUps

	}	

	//set up light type and position for lit_color_texture_program:
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	if(game.Object[189].name == "") MainLoop->volume = 1.0f;
	else MainLoop->volume = 0.0f;


	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));


		constexpr float H = 0.09f;
		float ofs = 850.0f / drawable_size.y;

			lines.draw_text(game.Object[189].name, // Main Msg
			glm::vec3(-aspect + 0.1f * H + 1.225f * ofs, -1.0 + 0.1f * H + 1.25f * ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xA0, 0xB8, 0x08, 0x00));
			
			lines.draw_text(game.Object[25].name, // P1 score
			glm::vec3(-aspect + 0.1f * H + 0.3f * ofs, -1.0 + 0.1f * H + 1.0f * ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xA0, 0xB8, 0xC8, 0x00));

			lines.draw_text(game.Object[26].name, // P2 score
			glm::vec3(-aspect + 0.1f * H + 2.25f * ofs, -1.0 + 0.1f * H + 1.0f * ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xA0, 0xB8, 0x08, 0x00));

			if(game.Object[189].name == "") {
				lines.draw_text(game.Object[208].name, // P1 Color PUP
				glm::vec3(-aspect + 0.1f * H + 0.2f * ofs, -1.0 + 0.1f * H + 1.20f * ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xA0, 0xB8, 0xC8, 0x00));

				lines.draw_text(game.Object[209].name, // P1 Reverse PUP
				glm::vec3(-aspect + 0.1f * H + 0.3f * ofs, -1.0 + 0.1f * H + 1.45f * ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xA0, 0xB8, 0xC8, 0x00));
				//
				lines.draw_text(game.Object[210].name, // P2 Color PUP
				glm::vec3(-aspect + 0.1f * H + 2.1f * ofs, -1.0 + 0.1f * H + 1.20f * ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xA0, 0xB8, 0x08, 0x00));
				//
				lines.draw_text(game.Object[211].name, // P2 Reverse PUP
				glm::vec3(-aspect + 0.1f * H + 2.25f * ofs, -1.0 + 0.1f * H + 1.45f * ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xA0, 0xB8, 0x08, 0x00));
			}
	}
	GL_ERRORS();
}
