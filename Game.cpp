#include "Game.hpp"
//#include "PlayMode.hpp"
#include "Connection.hpp"
#include "Mesh.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

#include <glm/gtx/norm.hpp>

// Include Meshes





void Player::Controls::send_controls_message(Connection *connection_) const {
	assert(connection_);
	auto &connection = *connection_;

	uint32_t size = 5;
	connection.send(Message::C2S_Controls);
	connection.send(uint8_t(size));
	connection.send(uint8_t(size >> 8));
	connection.send(uint8_t(size >> 16));

	auto send_button = [&](Button const &b) {
		if (b.downs & 0x80) {
			std::cerr << "Wow, you are really good at pressing buttons!" << std::endl;
		}
		connection.send(uint8_t( (b.pressed ? 0x80 : 0x00) | (b.downs & 0x7f) ) );
	};

	send_button(left);
	send_button(right);
	send_button(up);
	send_button(down);
	send_button(space);
}

bool Player::Controls::recv_controls_message(Connection *connection_) {
	assert(connection_);
	auto &connection = *connection_;

	auto &recv_buffer = connection.recv_buffer;

	//expecting [type, size_low0, size_mid8, size_high8]:
	if (recv_buffer.size() < 4) return false;
	if (recv_buffer[0] != uint8_t(Message::C2S_Controls)) return false;
	uint32_t size = (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	if (size != 5) throw std::runtime_error("Controls message with size " + std::to_string(size) + " != 5!");
	
	//expecting complete message:
	if (recv_buffer.size() < 4 + size) return false;

	auto recv_button = [](uint8_t byte, Button *button) {
		button->pressed = (byte & 0x80);
		uint32_t d = uint32_t(button->downs) + uint32_t(byte & 0x7f);
		if (d > 255) {
			std::cerr << "got a whole lot of downs" << std::endl;
			d = 255;
		}
		button->downs = uint8_t(d);
	};

	recv_button(recv_buffer[4+0], &left);
	recv_button(recv_buffer[4+1], &right);
	recv_button(recv_buffer[4+2], &up);
	recv_button(recv_buffer[4+3], &down);
	recv_button(recv_buffer[4+4], &space);

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + size);
	//printf("PASS");
	return true;
}


//-----------------------------------------

void Game::hide_object(Player *obj){
	obj->position.x = -500.0f;
	obj->position.y = -500.0f;
	obj->velocity = glm::vec2(0.0f, 0.0f);
}


// Code referenced from : https://www.toptal.com/game/video-game-physics-part-ii-collision-detection-for-solid-objects#:~:text=Collision%20Physics%20in%20Video%20Games,falls%20below%20a%20small%20tolerance.
bool Game::check_overlap(Player *obj1, glm::vec2 size1, Player *obj2, glm::vec2 size2){

	float obj1_min_x = obj1->position.x - size1.x/2.0f;
	float obj1_max_x = obj1->position.x + size1.x/2.0f;
	float obj1_min_y = obj1->position.y - size1.y/2.0f;
	float obj1_max_y = obj1->position.y + size1.y/2.0f;

	float obj2_min_x = obj2->position.x - size2.x/2.0f;
	float obj2_max_x = obj2->position.x + size2.x/2.0f;
	float obj2_min_y = obj2->position.y - size2.y/2.0f;
	float obj2_max_y = obj2->position.y + size2.y/2.0f;

    float d1x = obj2_min_x - obj1_max_x;
    float d1y = obj2_min_y - obj1_max_y;
    float d2x = obj1_min_x - obj2_max_x;
    float d2y = obj1_min_y - obj2_max_y;

    if (d1x > 0.0f || d1y > 0.0f)
        return false;

    if (d2x > 0.0f || d2y > 0.0f)
        return false;

    return true;
}

void Game::swap_ball(Player *obj, int p) {
	if(p == 0) // Swap to P1 ball
	{
		for(uint8_t i = 2; i < 12; i++){
			if(Object[i].position.x == -500.0f) {// If avaiable
				glm::vec2 vel = obj->velocity;
				glm::vec2 pos = obj->position;
				obj->velocity = Object[i].velocity;
				obj->position = Object[i].position;
				Object[i].velocity = vel;
				Object[i].position = pos;		
				break;
			}
		}
	}
	else // Swap to P2 ball
	{
		for(uint8_t i = 12; i < 22; i++){
			if(Object[i].position.x == -500.0f) {// If avaiable
				glm::vec2 vel = obj->velocity;
				glm::vec2 pos = obj->position;
				obj->velocity = Object[i].velocity;
				obj->position = Object[i].position;
				Object[i].velocity = vel;
				Object[i].position = pos;		
				break;
			}
		}
	}

}


void Game::init(int state) {


	// Assign Object Positions and names -
	Object[0].position.x = 0.000000f; Object[0].position.y = -65.000000f; Object[0].name = "P1_Paddel"; Object[1].position.x = 0.000000f; Object[1].position.y = 65.000000f; Object[1].name = "P2_Paddel"; Object[2].position.x = 0.000000f; Object[2].position.y = -63.000000f; Object[2].name = "Ball.001"; Object[3].position.x = 347.933044f; Object[3].position.y = 43.500000f; Object[3].name = "Ball.002"; Object[4].position.x = 361.725769f; Object[4].position.y = 43.500000f; Object[4].name = "Ball.003"; Object[5].position.x = 361.725769f; Object[5].position.y = -43.500000f; Object[5].name = "Ball.004"; Object[6].position.x = 376.173004f; Object[6].position.y = 43.500000f; Object[6].name = "Ball.005"; Object[7].position.x = 376.173004f; Object[7].position.y = -43.500000f; Object[7].name = "Ball.006"; Object[8].position.x = 390.620209f; Object[8].position.y = 43.500000f; Object[8].name = "Ball.007"; Object[9].position.x = 390.620209f; Object[9].position.y = -43.500000f; Object[9].name = "Ball.008";
	Object[10].position.x = 405.067444f; Object[10].position.y = 43.500000f; Object[10].name = "Ball.009"; Object[11].position.x = 405.066986f; Object[11].position.y = -43.500000f; Object[11].name = "Ball.010"; Object[12].position.x = 0.000000f; Object[12].position.y = 63.000000f; Object[12].name = "Ball.011"; Object[13].position.x = 416.243591f; Object[13].position.y = -43.500000f; Object[13].name = "Ball.012"; Object[14].position.x = 430.690826f; Object[14].position.y = 43.500000f; Object[14].name = "Ball.013"; Object[15].position.x = 430.690826f; Object[15].position.y = -43.500000f; Object[15].name = "Ball.014"; Object[16].position.x = 445.138031f; Object[16].position.y = 43.500000f; Object[16].name = "Ball.015"; Object[17].position.x = 445.138031f; Object[17].position.y = -43.500000f; Object[17].name = "Ball.016"; Object[18].position.x = 459.585266f; Object[18].position.y = 43.500000f; Object[18].name = "Ball.017"; Object[19].position.x = 459.585266f; Object[19].position.y = -43.500000f; Object[19].name = "Ball.018";
	Object[20].position.x = 470.488831f; Object[20].position.y = 43.500000f; Object[20].name = "Ball.019"; Object[21].position.x = 470.488831f; Object[21].position.y = -43.500000f; Object[21].name = "Ball.020"; Object[22].position.x = 10.250000f; Object[22].position.y = -1.500000f; Object[22].name = "P1_Brick.001"; Object[23].position.x = 20.500000f; Object[23].position.y = -1.500000f; Object[23].name = "P1_Brick.002"; Object[24].position.x = 30.750000f; Object[24].position.y = -1.500000f; Object[24].name = "P1_Brick.003"; Object[25].position.x = -30.750000f; Object[25].position.y = -1.500000f; Object[25].name = "P1_Brick.004"; Object[26].position.x = -20.500000f; Object[26].position.y = -1.500000f; Object[26].name = "P1_Brick.005"; Object[27].position.x = -10.250000f; Object[27].position.y = -1.500000f; Object[27].name = "P1_Brick.006"; Object[28].position.x = 30.750000f; Object[28].position.y = -3.750000f; Object[28].name = "P1_Brick.007"; Object[29].position.x = 20.500000f; Object[29].position.y = -3.750000f; Object[29].name = "P1_Brick.008";
	Object[30].position.x = 10.250000f; Object[30].position.y = -3.750000f; Object[30].name = "P1_Brick.009"; Object[31].position.x = 0.000000f; Object[31].position.y = -3.750000f; Object[31].name = "P1_Brick.010"; Object[32].position.x = -10.250000f; Object[32].position.y = -3.750000f; Object[32].name = "P1_Brick.011"; Object[33].position.x = -30.750000f; Object[33].position.y = -3.750000f; Object[33].name = "P1_Brick.012"; Object[34].position.x = -20.500000f; Object[34].position.y = -3.750000f; Object[34].name = "P1_Brick.013"; Object[35].position.x = 0.000000f; Object[35].position.y = -6.000000f; Object[35].name = "P1_Brick.014"; Object[36].position.x = 10.250000f; Object[36].position.y = -6.000000f; Object[36].name = "P1_Brick.015"; Object[37].position.x = 20.500000f; Object[37].position.y = -6.000000f; Object[37].name = "P1_Brick.016"; Object[38].position.x = 30.750000f; Object[38].position.y = -6.000000f; Object[38].name = "P1_Brick.017"; Object[39].position.x = -30.750000f; Object[39].position.y = -6.000000f; Object[39].name = "P1_Brick.018";
	Object[40].position.x = -20.500000f; Object[40].position.y = -6.000000f; Object[40].name = "P1_Brick.019"; Object[41].position.x = -10.250000f; Object[41].position.y = -6.000000f; Object[41].name = "P1_Brick.020"; Object[42].position.x = 30.750000f; Object[42].position.y = -8.250000f; Object[42].name = "P1_Brick.021"; Object[43].position.x = 20.500000f; Object[43].position.y = -8.250000f; Object[43].name = "P1_Brick.022"; Object[44].position.x = 10.250000f; Object[44].position.y = -8.250000f; Object[44].name = "P1_Brick.023"; Object[45].position.x = 0.000000f; Object[45].position.y = -8.250000f; Object[45].name = "P1_Brick.024"; Object[46].position.x = -10.250000f; Object[46].position.y = -8.250000f; Object[46].name = "P1_Brick.025"; Object[47].position.x = -30.750000f; Object[47].position.y = -8.250000f; Object[47].name = "P1_Brick.026"; Object[48].position.x = -20.500000f; Object[48].position.y = -8.250000f; Object[48].name = "P1_Brick.027"; Object[49].position.x = 0.000000f; Object[49].position.y = -10.500000f; Object[49].name = "P1_Brick.028";
	Object[50].position.x = 10.250000f; Object[50].position.y = -10.500000f; Object[50].name = "P1_Brick.029"; Object[51].position.x = 20.500000f; Object[51].position.y = -10.500000f; Object[51].name = "P1_Brick.030"; Object[52].position.x = 30.750000f; Object[52].position.y = -10.500000f; Object[52].name = "P1_Brick.031"; Object[53].position.x = -30.750000f; Object[53].position.y = -10.500000f; Object[53].name = "P1_Brick.032"; Object[54].position.x = -20.500000f; Object[54].position.y = -10.500000f; Object[54].name = "P1_Brick.033"; Object[55].position.x = -10.250000f; Object[55].position.y = -10.500000f; Object[55].name = "P1_Brick.034"; Object[56].position.x = 30.750000f; Object[56].position.y = -12.750000f; Object[56].name = "P1_Brick.035"; Object[57].position.x = 20.500000f; Object[57].position.y = -12.750000f; Object[57].name = "P1_Brick.036"; Object[58].position.x = 10.250000f; Object[58].position.y = -12.750000f; Object[58].name = "P1_Brick.037"; Object[59].position.x = 0.000000f; Object[59].position.y = -12.750000f; Object[59].name = "P1_Brick.038";
	Object[60].position.x = -10.250000f; Object[60].position.y = -12.750000f; Object[60].name = "P1_Brick.039"; Object[61].position.x = -30.750000f; Object[61].position.y = -12.750000f; Object[61].name = "P1_Brick.040"; Object[62].position.x = -20.500000f; Object[62].position.y = -12.750000f; Object[62].name = "P1_Brick.041"; Object[63].position.x = 0.000000f; Object[63].position.y = -1.500000f; Object[63].name = "P1_Brick.042"; Object[64].position.x = 0.000000f; Object[64].position.y = -15.000000f; Object[64].name = "P1_Brick.043"; Object[65].position.x = 10.250000f; Object[65].position.y = -15.000000f; Object[65].name = "P1_Brick.044"; Object[66].position.x = 20.500000f; Object[66].position.y = -15.000000f; Object[66].name = "P1_Brick.045"; Object[67].position.x = 30.750000f; Object[67].position.y = -15.000000f; Object[67].name = "P1_Brick.046"; Object[68].position.x = -30.750000f; Object[68].position.y = -15.000000f; Object[68].name = "P1_Brick.047"; Object[69].position.x = -20.500000f; Object[69].position.y = -15.000000f; Object[69].name = "P1_Brick.048";
	Object[70].position.x = -10.250000f; Object[70].position.y = -15.000000f; Object[70].name = "P1_Brick.049"; Object[71].position.x = 30.750000f; Object[71].position.y = -17.250000f; Object[71].name = "P1_Brick.050"; Object[72].position.x = 20.500000f; Object[72].position.y = -17.250000f; Object[72].name = "P1_Brick.051"; Object[73].position.x = 10.250000f; Object[73].position.y = -17.250000f; Object[73].name = "P1_Brick.052"; Object[74].position.x = 0.000000f; Object[74].position.y = -17.250000f; Object[74].name = "P1_Brick.053"; Object[75].position.x = -10.250000f; Object[75].position.y = -17.250000f; Object[75].name = "P1_Brick.054"; Object[76].position.x = -30.750000f; Object[76].position.y = -17.250000f; Object[76].name = "P1_Brick.055"; Object[77].position.x = -20.500000f; Object[77].position.y = -17.250000f; Object[77].name = "P1_Brick.056"; Object[78].position.x = 0.000000f; Object[78].position.y = -19.500000f; Object[78].name = "P1_Brick.057"; Object[79].position.x = 10.250000f; Object[79].position.y = -19.500000f; Object[79].name = "P1_Brick.058";
	Object[80].position.x = 20.500000f; Object[80].position.y = -19.500000f; Object[80].name = "P1_Brick.059"; Object[81].position.x = 30.750000f; Object[81].position.y = -19.500000f; Object[81].name = "P1_Brick.060"; Object[82].position.x = -30.750000f; Object[82].position.y = -19.500000f; Object[82].name = "P1_Brick.061"; Object[83].position.x = -20.500000f; Object[83].position.y = -19.500000f; Object[83].name = "P1_Brick.062"; Object[84].position.x = -10.250000f; Object[84].position.y = -19.500000f; Object[84].name = "P1_Brick.063"; Object[85].position.x = 30.750000f; Object[85].position.y = -21.750000f; Object[85].name = "P1_Brick.064"; Object[86].position.x = 20.500000f; Object[86].position.y = -21.750000f; Object[86].name = "P1_Brick.065"; Object[87].position.x = 10.250000f; Object[87].position.y = -21.750000f; Object[87].name = "P1_Brick.066"; Object[88].position.x = 0.000000f; Object[88].position.y = -21.750000f; Object[88].name = "P1_Brick.067"; Object[89].position.x = -10.250000f; Object[89].position.y = -21.750000f; Object[89].name = "P1_Brick.068";
	Object[90].position.x = -30.750000f; Object[90].position.y = -21.750000f; Object[90].name = "P1_Brick.069"; Object[91].position.x = -20.500000f; Object[91].position.y = -21.750000f; Object[91].name = "P1_Brick.070"; Object[92].position.x = 0.000000f; Object[92].position.y = -24.000000f; Object[92].name = "P1_Brick.071"; Object[93].position.x = 10.250000f; Object[93].position.y = -24.000000f; Object[93].name = "P1_Brick.072"; Object[94].position.x = 20.500000f; Object[94].position.y = -24.000000f; Object[94].name = "P1_Brick.073"; Object[95].position.x = 30.750000f; Object[95].position.y = -24.000000f; Object[95].name = "P1_Brick.074"; Object[96].position.x = -30.750000f; Object[96].position.y = -24.000000f; Object[96].name = "P1_Brick.075"; Object[97].position.x = -20.500000f; Object[97].position.y = -24.000000f; Object[97].name = "P1_Brick.076"; Object[98].position.x = -10.250000f; Object[98].position.y = -24.000000f; Object[98].name = "P1_Brick.077"; Object[99].position.x = 30.750000f; Object[99].position.y = -26.250000f; Object[99].name = "P1_Brick.078";
	Object[100].position.x = 20.500000f; Object[100].position.y = -26.250000f; Object[100].name = "P1_Brick.079"; Object[101].position.x = 10.250000f; Object[101].position.y = -26.250000f; Object[101].name = "P1_Brick.080"; Object[102].position.x = 0.000000f; Object[102].position.y = -26.250000f; Object[102].name = "P1_Brick.081"; Object[103].position.x = -10.250000f; Object[103].position.y = -26.250000f; Object[103].name = "P1_Brick.082"; Object[104].position.x = -30.750000f; Object[104].position.y = -26.250000f; Object[104].name = "P1_Brick.083"; Object[105].position.x = -20.500000f; Object[105].position.y = -26.250000f; Object[105].name = "P1_Brick.084"; Object[106].position.x = 0.000001f; Object[106].position.y = 26.250002f; Object[106].name = "P2_Brick.001"; Object[107].position.x = -20.500000f; Object[107].position.y = 26.250000f; Object[107].name = "P2_Brick.002"; Object[108].position.x = -30.750000f; Object[108].position.y = 26.250000f; Object[108].name = "P2_Brick.003"; Object[109].position.x = 30.750000f; Object[109].position.y = 26.250002f; Object[109].name = "P2_Brick.004";
	Object[110].position.x = 20.500000f; Object[110].position.y = 26.250004f; Object[110].name = "P2_Brick.005"; Object[111].position.x = 10.250000f; Object[111].position.y = 26.250004f; Object[111].name = "P2_Brick.006"; Object[112].position.x = -30.750000f; Object[112].position.y = 24.000000f; Object[112].name = "P2_Brick.007"; Object[113].position.x = -20.500000f; Object[113].position.y = 24.000000f; Object[113].name = "P2_Brick.008"; Object[114].position.x = -10.250000f; Object[114].position.y = 24.000000f; Object[114].name = "P2_Brick.009"; Object[115].position.x = 0.000001f; Object[115].position.y = 24.000002f; Object[115].name = "P2_Brick.010"; Object[116].position.x = 10.250000f; Object[116].position.y = 24.000004f; Object[116].name = "P2_Brick.011"; Object[117].position.x = 30.750000f; Object[117].position.y = 24.000002f; Object[117].name = "P2_Brick.012"; Object[118].position.x = 20.500000f; Object[118].position.y = 24.000004f; Object[118].name = "P2_Brick.013"; Object[119].position.x = 0.000001f; Object[119].position.y = 21.750002f; Object[119].name = "P2_Brick.014";
	Object[120].position.x = -10.250000f; Object[120].position.y = 21.750000f; Object[120].name = "P2_Brick.015"; Object[121].position.x = -20.500000f; Object[121].position.y = 21.750000f; Object[121].name = "P2_Brick.016"; Object[122].position.x = -30.750000f; Object[122].position.y = 21.750000f; Object[122].name = "P2_Brick.017"; Object[123].position.x = 30.750000f; Object[123].position.y = 21.750002f; Object[123].name = "P2_Brick.018"; Object[124].position.x = 20.500000f; Object[124].position.y = 21.750004f; Object[124].name = "P2_Brick.019"; Object[125].position.x = 10.250000f; Object[125].position.y = 21.750004f; Object[125].name = "P2_Brick.020"; Object[126].position.x = -30.750000f; Object[126].position.y = 19.500000f; Object[126].name = "P2_Brick.021"; Object[127].position.x = -20.500000f; Object[127].position.y = 19.500000f; Object[127].name = "P2_Brick.022"; Object[128].position.x = -10.250000f; Object[128].position.y = 19.500000f; Object[128].name = "P2_Brick.023"; Object[129].position.x = 0.000000f; Object[129].position.y = 19.500000f; Object[129].name = "P2_Brick.024";
	Object[130].position.x = 10.250000f; Object[130].position.y = 19.500000f; Object[130].name = "P2_Brick.025"; Object[131].position.x = 30.750000f; Object[131].position.y = 19.500000f; Object[131].name = "P2_Brick.026"; Object[132].position.x = 20.500000f; Object[132].position.y = 19.500000f; Object[132].name = "P2_Brick.027"; Object[133].position.x = 0.000000f; Object[133].position.y = 17.250002f; Object[133].name = "P2_Brick.028"; Object[134].position.x = -10.250000f; Object[134].position.y = 17.250000f; Object[134].name = "P2_Brick.029"; Object[135].position.x = -20.500000f; Object[135].position.y = 17.250000f; Object[135].name = "P2_Brick.030"; Object[136].position.x = -30.750000f; Object[136].position.y = 17.250000f; Object[136].name = "P2_Brick.031"; Object[137].position.x = 30.750000f; Object[137].position.y = 17.250002f; Object[137].name = "P2_Brick.032"; Object[138].position.x = 20.500000f; Object[138].position.y = 17.250004f; Object[138].name = "P2_Brick.033"; Object[139].position.x = 10.250000f; Object[139].position.y = 17.250004f; Object[139].name = "P2_Brick.034";
	Object[140].position.x = -30.750000f; Object[140].position.y = 14.999993f; Object[140].name = "P2_Brick.035"; Object[141].position.x = -20.500000f; Object[141].position.y = 14.999994f; Object[141].name = "P2_Brick.036"; Object[142].position.x = -10.250000f; Object[142].position.y = 14.999995f; Object[142].name = "P2_Brick.037"; Object[143].position.x = 0.000000f; Object[143].position.y = 14.999996f; Object[143].name = "P2_Brick.038"; Object[144].position.x = 10.250000f; Object[144].position.y = 14.999997f; Object[144].name = "P2_Brick.039"; Object[145].position.x = 30.750000f; Object[145].position.y = 15.000000f; Object[145].name = "P2_Brick.040"; Object[146].position.x = 20.500000f; Object[146].position.y = 14.999998f; Object[146].name = "P2_Brick.041"; Object[147].position.x = -0.000000f; Object[147].position.y = 12.750003f; Object[147].name = "P2_Brick.042"; Object[148].position.x = -10.250000f; Object[148].position.y = 12.750002f; Object[148].name = "P2_Brick.043"; Object[149].position.x = -20.500000f; Object[149].position.y = 12.750001f; Object[149].name = "P2_Brick.044";
	Object[150].position.x = -30.750000f; Object[150].position.y = 12.750000f; Object[150].name = "P2_Brick.045"; Object[151].position.x = 30.750000f; Object[151].position.y = 12.740006f; Object[151].name = "P2_Brick.046"; Object[152].position.x = 20.500000f; Object[152].position.y = 12.750005f; Object[152].name = "P2_Brick.047"; Object[153].position.x = 10.250000f; Object[153].position.y = 12.750004f; Object[153].name = "P2_Brick.048"; Object[154].position.x = -30.750000f; Object[154].position.y = 10.500000f; Object[154].name = "P2_Brick.049"; Object[155].position.x = -20.500000f; Object[155].position.y = 10.500001f; Object[155].name = "P2_Brick.050"; Object[156].position.x = -10.250000f; Object[156].position.y = 10.500002f; Object[156].name = "P2_Brick.051"; Object[157].position.x = -0.000000f; Object[157].position.y = 10.500003f; Object[157].name = "P2_Brick.052"; Object[158].position.x = 10.250000f; Object[158].position.y = 10.500004f; Object[158].name = "P2_Brick.053"; Object[159].position.x = 30.750000f; Object[159].position.y = 10.500007f; Object[159].name = "P2_Brick.054";
	Object[160].position.x = 20.500000f; Object[160].position.y = 10.500005f; Object[160].name = "P2_Brick.055"; Object[161].position.x = -0.000001f; Object[161].position.y = 8.250003f; Object[161].name = "P2_Brick.056"; Object[162].position.x = -10.250000f; Object[162].position.y = 8.250002f; Object[162].name = "P2_Brick.057"; Object[163].position.x = -20.500000f; Object[163].position.y = 8.250001f; Object[163].name = "P2_Brick.058"; Object[164].position.x = -30.750000f; Object[164].position.y = 8.250000f; Object[164].name = "P2_Brick.059"; Object[165].position.x = 30.750000f; Object[165].position.y = 8.250006f; Object[165].name = "P2_Brick.060"; Object[166].position.x = 20.500000f; Object[166].position.y = 8.250005f; Object[166].name = "P2_Brick.061"; Object[167].position.x = 10.250000f; Object[167].position.y = 8.250004f; Object[167].name = "P2_Brick.062"; Object[168].position.x = -30.750000f; Object[168].position.y = 6.000000f; Object[168].name = "P2_Brick.063"; Object[169].position.x = -20.500000f; Object[169].position.y = 6.000000f; Object[169].name = "P2_Brick.064";
	Object[170].position.x = -10.250000f; Object[170].position.y = 6.000000f; Object[170].name = "P2_Brick.065"; Object[171].position.x = -0.000001f; Object[171].position.y = 6.000001f; Object[171].name = "P2_Brick.066"; Object[172].position.x = 10.250000f; Object[172].position.y = 6.000003f; Object[172].name = "P2_Brick.067"; Object[173].position.x = 30.750000f; Object[173].position.y = 6.000003f; Object[173].name = "P2_Brick.068"; Object[174].position.x = 20.500000f; Object[174].position.y = 6.000003f; Object[174].name = "P2_Brick.069"; Object[175].position.x = -0.000001f; Object[175].position.y = 3.749998f; Object[175].name = "P2_Brick.070"; Object[176].position.x = -10.250000f; Object[176].position.y = 3.749996f; Object[176].name = "P2_Brick.071"; Object[177].position.x = -20.500000f; Object[177].position.y = 3.749996f; Object[177].name = "P2_Brick.072"; Object[178].position.x = -30.750000f; Object[178].position.y = 3.749996f; Object[178].name = "P2_Brick.073"; Object[179].position.x = 30.750000f; Object[179].position.y = 3.750000f; Object[179].name = "P2_Brick.074";
	Object[180].position.x = 20.500000f; Object[180].position.y = 3.750000f; Object[180].name = "P2_Brick.075"; Object[181].position.x = 10.250000f; Object[181].position.y = 3.750000f; Object[181].name = "P2_Brick.076"; Object[182].position.x = -30.750000f; Object[182].position.y = 1.500000f; Object[182].name = "P2_Brick.077"; Object[183].position.x = -20.500000f; Object[183].position.y = 1.500000f; Object[183].name = "P2_Brick.078"; Object[184].position.x = -10.250000f; Object[184].position.y = 1.500000f; Object[184].name = "P2_Brick.079"; Object[185].position.x = -0.000001f; Object[185].position.y = 1.500002f; Object[185].name = "P2_Brick.080"; Object[186].position.x = 10.250000f; Object[186].position.y = 1.500004f; Object[186].name = "P2_Brick.081"; Object[187].position.x = 30.750000f; Object[187].position.y = 1.500004f; Object[187].name = "P2_Brick.082"; Object[188].position.x = 20.500000f; Object[188].position.y = 1.500004f; Object[188].name = "P2_Brick.083"; Object[189].position.x = -10.250000f; Object[189].position.y = 26.250000f; Object[189].name = "P2_Brick.084";
	Object[190].position.x = 233.707245f; Object[190].position.y = -65.000000f; Object[190].name = "P1_Longer_Paddel"; Object[191].position.x = 217.693512f; Object[191].position.y = 65.000000f; Object[191].name = "P2_Longer_Paddel"; Object[192].position.x = 244.342010f; Object[192].position.y = -26.250000f; Object[192].name = "Power_Up.001"; Object[193].position.x = 264.192078f; Object[193].position.y = -26.250000f; Object[193].name = "Power_Up.002"; Object[194].position.x = 275.891785f; Object[194].position.y = -26.250000f; Object[194].name = "Power_Up.003"; Object[195].position.x = 295.741821f; Object[195].position.y = -26.250000f; Object[195].name = "Power_Up.004"; Object[196].position.x = 308.493225f; Object[196].position.y = -26.250000f; Object[196].name = "Power_Up.005"; Object[197].position.x = 328.343292f; Object[197].position.y = -26.250000f; Object[197].name = "Power_Up.006"; Object[198].position.x = 340.042999f; Object[198].position.y = -26.250000f; Object[198].name = "Power_Up.007"; Object[199].position.x = 359.893036f; Object[199].position.y = -26.250000f; Object[199].name = "Power_Up.008";
	Object[200].position.x = 244.342010f; Object[200].position.y = -16.653610f; Object[200].name = "Power_Up.009"; Object[201].position.x = 264.192078f; Object[201].position.y = -16.653610f; Object[201].name = "Power_Up.010"; Object[202].position.x = 275.891785f; Object[202].position.y = -16.653610f; Object[202].name = "Power_Up.011"; Object[203].position.x = 295.741821f; Object[203].position.y = -16.653610f; Object[203].name = "Power_Up.012"; Object[204].position.x = 308.493225f; Object[204].position.y = -16.653610f; Object[204].name = "Power_Up.013"; Object[205].position.x = 328.343292f; Object[205].position.y = -16.653610f; Object[205].name = "Power_Up.014"; Object[206].position.x = 340.042999f; Object[206].position.y = -16.653610f; Object[206].name = "Power_Up.015"; Object[207].position.x = 359.893036f; Object[207].position.y = -16.653610f; Object[207].name = "Power_Up.016"; Object[208].position.x = 244.342010f; Object[208].position.y = -10.738028f; Object[208].name = "Power_Up.017"; Object[209].position.x = 264.192078f; Object[209].position.y = -10.738028f; Object[209].name = "Power_Up.018";
	Object[210].position.x = 275.891785f; Object[210].position.y = -10.738028f; Object[210].name = "Power_Up.019"; Object[211].position.x = 295.741821f; Object[211].position.y = -10.738028f; Object[211].name = "Power_Up.020";	
	

	
	
	// Using Object[189] str for Main Msg
	// Using Object[25] str for Player 1 score
	// Using Object[26] str for Player 2 score
	// Using Object[211] str for Reverse P2
	// Using Object[210] str for Switch Color P2
	// Using Object[209] str for Reverse P1
	// Using Object[208] str for Switch Color P1

	if(state == 0)
		Object[189].name = "Press Space to Begin";
	if(state == 1) {
		if(P1_score > P2_score)
				Object[189].name = "Game Over: P1 Wins";
		else if(P1_score < P2_score)
			Object[189].name = "Game Over: P2 Wins";
		if(P1_score == P2_score)
				Object[189].name = "Game Over: Draw";
	}
	
	
	
	P1_Ball_Spawn_Loc = Object[1 + 2 - 1].position;
	P2_Ball_Spawn_Loc = Object[11 + 2 - 1].position;
	 
	P1_score = 0; P2_score = 0;
	begin_game = 0;
	P1_long = 0; P2_long = 0;
	P1_fast = 0; P2_fast = 0;

	P1_Speed = 50.0f;
	P2_Speed = 50.0f;
	ball_speed = 75.0f;
	powerup_speed = 50.0f;

	P1_powerup_bitmap[0] = 0;
	P2_powerup_bitmap[0] = 0;
	P1_powerup_bitmap[1] = 0;
	P2_powerup_bitmap[1] = 0;
	P1_powerup_bitmap[2] = 0;
	P2_powerup_bitmap[2] = 0;
	P1_powerup_bitmap[3] = 0;
	P2_powerup_bitmap[3] = 0;
	P1_powerup_bitmap[4] = 0;
	P2_powerup_bitmap[4] = 0;

	paddle_dim = glm::vec2(15.0f, 2.0f);
	brick_dim =  glm::vec2(10.0f, 2.0f);
	ball_dim =  glm::vec2(2.0f, 2.0f);
	powerup_dim = glm::vec2(10.0f, 2.0f);
	longpaddle_dim = glm::vec2(20.0f, 2.0f);
	horizontal_bounds = glm::vec2(-40.0f, 40.0f);
	vertical_bounds = glm::vec2(-80.0f, 80.0f);

	Object[25].name = "";
	Object[26].name = "";

	Object[211].name = "";
	Object[210].name = "";

	Object[209].name = "";
	Object[208].name = "";


	// Hide all balls and Power Ups
	for(uint32_t i = 0; i < 212; i++) {
		if(i >= 2 && i <= 21) {
			if(i == 2 || i == 12) continue;
			hide_object(&Object[i]);
		}  // Balls
		if(i >= 190) hide_object(&Object[i]); // Power Ups and Long Paddles
	}	

	for(uint32_t i = 0; i < 20; i++) {
		all_brick_dir[i] = 0;
	}



}
Game::Game() : mt(0x15466666) {
}

Player *Game::spawn_player() {
	players.emplace_back();
	Player &player = players.back();

	//random point in the middle area of the arena:
	player.position.x = glm::mix(ArenaMin.x + 2.0f * PlayerRadius, ArenaMax.x - 2.0f * PlayerRadius, 0.4f + 0.2f * mt() / float(mt.max()));
	player.position.y = glm::mix(ArenaMin.y + 2.0f * PlayerRadius, ArenaMax.y - 2.0f * PlayerRadius, 0.4f + 0.2f * mt() / float(mt.max()));

	do {
		player.color.r = mt() / float(mt.max());
		player.color.g = mt() / float(mt.max());
		player.color.b = mt() / float(mt.max());
	} while (player.color == glm::vec3(0.0f));
	player.color = glm::normalize(player.color);

	player.name = "Player " + std::to_string(next_player_number++);

	return &player;
}

void Game::remove_player(Player *player) {
	bool found = false;

	// Object[0] and Object[1] check 
	if(&Object[0] == player) {
		found = true;
		P1_assigned = 0;

	}
	if(&Object[1] == player) {
		found = true;
		P2_assigned = 0;
	}

	for (auto pi = players.begin(); pi != players.end(); ++pi) {
		if (&*pi == player) {
			players.erase(pi);
			found = true;
			break;
		}
	}
	assert(found);
}

Player *Game::assign_player() {
	if(P1_assigned == 0) {
		P1_assigned = 1;
		init(0);
		return &Object[0];

	}
	else if(P2_assigned == 0) {
		P2_assigned = 1;
		return &Object[1];
	}
	else exit(0); // No more than 2 players at a time
}

void Game::update(float elapsed) {
	// =========================
	// Begin Game
	// =========================
	if(((Object[0].controls.space.pressed == 1) ||  (Object[1].controls.space.pressed == 1)) && begin_game == 0) {
		begin_game = 1;
		Object[189].name = "";
		randnum = 1 + std::rand() % 10;

		// Give Random Initial Velocity
		float x = randnum/10.0f;
		if(x > 0.8f) x = 0.8f;
		float y = 1.0f - x;
		if(std::rand()%2 == 0)
		{
			Object[1 + 2 - 1].velocity.x = -1.0f*x*ball_speed; // Set initial velocity
		}
		else {
			Object[1 + 2 - 1].velocity.x = x*ball_speed; // Set initial velocity
		}

		if(std::rand()%2 == 0)
		{
			Object[11 + 2 - 1].velocity.x = -1.0f*x*ball_speed; // Set initial velocity
		}
		else {
			Object[11 + 2 - 1].velocity.x = x*ball_speed; // Set initial velocity
		}

		

		Object[1 + 2 - 1].velocity.y = y*ball_speed; // Set initial velocity
		Object[11 + 2 - 1].velocity.y = -y*ball_speed; // Set initial velocity
	}

	if(begin_game == 1) {

		// =========================
		// Paddle Movement
		// =========================
		int P1_id = 0;
		int P2_id = 1;
		paddle_dim_P1 = paddle_dim;
		paddle_dim_P2 = paddle_dim;
		if(P1_long == 1) {
			P1_id = 190;
			paddle_dim_P1 = longpaddle_dim;
			} // P1 is long
		if(P2_long == 1) {
			P2_id = 191;
			paddle_dim_P2 = longpaddle_dim;
			} // P1 is long

		// For Object[0]
		glm::vec2 dir = glm::vec2(0.0f, 0.0f);
		
		//glm::vec2 dir = glm::vec2(0.0f, 0.0f);
		if (Object[0].controls.left.pressed) dir.x -= 1.0f;
		if (Object[0].controls.right.pressed) dir.x += 1.0f;
		if (Object[0].controls.down.pressed) dir.y -= 1.0f;
		if (Object[0].controls.up.pressed) dir.y += 1.0f;

		// Bounds check
		float curr_pos = Object[P1_id].position.x;
		if(curr_pos + (dir.x * elapsed * P1_Speed) - (paddle_dim_P1.x/2.0f) <= horizontal_bounds.x) Object[P1_id].position.x = horizontal_bounds.x + (paddle_dim_P1.x/2.0f);
		else if(curr_pos + (dir.x * elapsed * P1_Speed) + (paddle_dim_P1.x/2.0f) >= horizontal_bounds.y) Object[P1_id].position.x = horizontal_bounds.y - (paddle_dim_P1.x/2.0f);
		else Object[P1_id].position.x += dir.x * elapsed * P1_Speed; // Update

		//reset 'downs' since controls have been handled:
		Object[0].controls.left.downs = 0;
		Object[0].controls.right.downs = 0;
		Object[0].controls.up.downs = 0;
		Object[0].controls.down.downs = 0;
		Object[0].controls.space.downs = 0;
		
		// For Object[1]
		
		dir = glm::vec2(0.0f, 0.0f);
		if (Object[1].controls.left.pressed) dir.x -= 1.0f;
		if (Object[1].controls.right.pressed) dir.x += 1.0f;
		if (Object[1].controls.down.pressed) dir.y -= 1.0f;
		if (Object[1].controls.up.pressed) dir.y += 1.0f;
		// Bounds check
		curr_pos = Object[P2_id].position.x;
		if(curr_pos + (dir.x * elapsed * P1_Speed) - (paddle_dim_P2.x/2.0f) <= horizontal_bounds.x) Object[P2_id].position.x = horizontal_bounds.x + (paddle_dim_P2.x/2.0f);
		else if(curr_pos + (dir.x * elapsed * P1_Speed) + (paddle_dim_P2.x/2.0f) >= horizontal_bounds.y) Object[P2_id].position.x = horizontal_bounds.y - (paddle_dim_P2.x/2.0f);
		else Object[P2_id].position.x += dir.x * elapsed * P2_Speed; // Update

		//reset 'downs' since controls have been handled:
		Object[1].controls.left.downs = 0;
		Object[1].controls.right.downs = 0;
		Object[1].controls.up.downs = 0;
		Object[1].controls.down.downs = 0;
		Object[1].controls.space.downs = 0;



		// ==================================
		// Ball Movement
		// ==================================
		for(uint8_t i = 0; i < 20; i++) {
			if(Object[i + 2].position.y != -500.0f && Object[i + 2].position.x != -500.0f)
			{
				Object[i + 2].position += Object[i + 2].velocity * elapsed;
			}
		}

		// ==================================
		// Ball Escape 
		// ==================================
		for(uint8_t i = 0; i < 20; i++) {
			if(Object[i + 2].position.y != -500.0f && Object[i + 2].position.x != -500.0f)
			{
				if (Object[i + 2].position.y <= vertical_bounds.x) // Behind P1, P2 score
				{
					P2_score += 1;
					hide_object(&Object[i+2]);
					////Object[i+2].velocity.y *= -1.0f;

				}
				else if (Object[i + 2].position.y >= vertical_bounds.y) // Behind P2, P1 score
				{
					P1_score += 1;
					hide_object(&Object[i+2]);
					////Object[i+2].velocity.y *= -1.0f;
				}
			}
		}

		// =============================
		// Ball Bound Reflection
		// =============================
		for(uint8_t i = 0; i < 20; i++) {
			if(Object[i + 2].position.y != -500.0f && Object[i + 2].position.x != -500.0f) { // Balls in Play Area

				if(Object[i + 2].position.x + Object[i + 2].velocity.x*elapsed - (ball_dim.x/2.0f) < horizontal_bounds.x) {
					Object[i + 2].velocity.x = -Object[i + 2].velocity.x;
					Object[i + 2].position.x = horizontal_bounds.x + (ball_dim.x/2.0f);
				}
				else if(Object[i + 2].position.x + Object[i + 2].velocity.x*elapsed + (ball_dim.x/2.0f) > horizontal_bounds.y) {
					Object[i + 2].velocity.x = -Object[i + 2].velocity.x;
					Object[i + 2].position.x = horizontal_bounds.y - (ball_dim.x/2.0f);
				}

			}
		}


		// ===========================
		// Paddle Collision
		// ===========================		
		for(uint8_t i = 0; i < 20; i++) {
			if(Object[i + 2].position.y != -500.0f && Object[i + 2].position.x != -500.0f) { // Balls in Play Area


				// Ball dims
				float Ball_cx = Object[i+2].position.x;
				//float Ball_cy = Object[i+2].position.y;
				float Ball_lx = Object[i+2].position.x - ball_dim.x/2.0f;
				float Ball_rx = Object[i+2].position.x + ball_dim.x/2.0f;
				float Ball_ty = Object[i+2].position.y + ball_dim.y/2.0f;
				float Ball_by = Object[i+2].position.y - ball_dim.y/2.0f;


				// P0 Check
				float P0_cx = Object[P1_id].position.x;
				float P0_lx = Object[P1_id].position.x - paddle_dim_P1.x/2.0f;
				float P0_rx = Object[P1_id].position.x + paddle_dim_P1.x/2.0f;
				//float P0_cy = Object[P1_id].position.y;
				float P0_by = Object[P1_id].position.y - paddle_dim_P1.y/2.0f;
				float P0_ty = Object[P1_id].position.y + paddle_dim_P1.y/2.0f;

				float x_speed = 0.0f;
				float y_speed = 0.0f;
				float overlap = 0.0f;

				if(check_overlap(&Object[P1_id], paddle_dim_P1, &Object[i+2], ball_dim)) {
					// Top Hit
					if(Ball_lx <= P0_rx && Ball_rx >= P0_lx && Ball_ty > P0_by) {
						x_speed = (Ball_cx - P0_cx)/(paddle_dim_P1.x/2.0f);
						if(std::abs(x_speed) > 0.9f) {
							if(x_speed < 0.0f) x_speed = -0.9f;
							else x_speed = 0.9f;
						}
						y_speed = 1.0f - std::abs(x_speed);
						Object[i + 2].velocity = glm::vec2(x_speed*ball_speed, y_speed*ball_speed); 
						// Push out logic 
						//overlap = std::abs(std::abs(Ball_by) - std::abs(P0_ty));
						//Object[i+2].position.y += overlap;
					}
					// Bottom Hit
					if(Ball_lx <= P0_rx && Ball_rx >= P0_lx && Ball_by < P0_ty){
						x_speed = (Ball_cx - P0_cx)/(paddle_dim_P1.x/2.0f);
						if(std::abs(x_speed) > 0.9f) {
							if(x_speed < 0.0f) x_speed = -0.9f;
							else x_speed = 0.9f;
						}
						y_speed = 1.0f - std::abs(x_speed);
						Object[i + 2].velocity = glm::vec2(x_speed*ball_speed, -1.0f*y_speed*ball_speed); 
						// Push out logic 
						//overlap = std::abs(std::abs(Ball_ty) - std::abs(P0_by));
						//Object[i+2].position.y -= overlap;
					}
					//// Right Hit
					//if(Ball_cx >= P0_rx){
					//	Object[i + 2].velocity.x *= -1.0f;
					//	//Push out logic
					//	overlap = std::abs(std::abs(Ball_lx) - std::abs(P0_rx));
					//	Object[i+2].position.x += overlap*P1_Speed*elapsed;
					//}
					//// Left Hit
					//if(Ball_cx <= P0_lx) {
					//	Object[i + 2].velocity.x *= -1.0f;
					//	//Push out logic
					//	overlap = std::abs(std::abs(Ball_rx) - std::abs(P0_lx));
					//	Object[i+2].position.x -= overlap*P1_Speed*elapsed;
					//}

					if(i + 2 >= 12) // P2 balls hitting P1 Paddle
						swap_ball(&Object[i+2], 0); // Swap to P1 Ball

				}

				// P1 Check
				float P1_cx = Object[P2_id].position.x;
				float P1_lx = Object[P2_id].position.x - paddle_dim_P2.x/2.0f;
				float P1_rx = Object[P2_id].position.x + paddle_dim_P2.x/2.0f;
				//float P1_cy = Object[P2_id].position.y;
				float P1_by = Object[P2_id].position.y - paddle_dim_P2.y/2.0f;
				float P1_ty = Object[P2_id].position.y + paddle_dim_P2.y/2.0f;

				x_speed = 0.0f;
				y_speed = 0.0f;
				overlap = 0.0f;

				
				if(check_overlap(&Object[P2_id], paddle_dim_P2, &Object[i+2], ball_dim)) {
					// Top Hit
					if(Ball_lx <= P1_rx && Ball_rx >= P1_lx && Ball_ty > P1_by) {
						x_speed = (Ball_cx - P1_cx)/(paddle_dim_P2.x/2.0f);
						
						if(std::abs(x_speed) > 0.9f) {
							if(x_speed < 0.0f) x_speed = -0.9f;
							else x_speed = 0.9f;
						}
						y_speed = 1.0f - std::abs(x_speed);
						Object[i + 2].velocity = glm::vec2(x_speed*ball_speed, y_speed*ball_speed); 
						// Push out logic 
						//overlap = std::abs(std::abs(Ball_by) - std::abs(P1_ty));
						//Object[i+2].position.y += overlap;
					}
					// Bottom Hit
					if(Ball_lx <= P1_rx && Ball_rx >= P1_lx && Ball_by < P1_ty){
						x_speed = (Ball_cx - P1_cx)/(paddle_dim_P2.x/2.0f);
						if(std::abs(x_speed) > 0.9f) {
							if(x_speed < 0.0f) x_speed = -0.9f;
							else x_speed = 0.9f;
						}
						y_speed = 1.0f - std::abs(x_speed);
						Object[i + 2].velocity = glm::vec2(x_speed*ball_speed, -1.0f*y_speed*ball_speed); 
						// Push out logic 
						//overlap = std::abs(std::abs(Ball_ty) - std::abs(P1_by));
						//Object[i+2].position.y -= overlap;
					}
					//// Right Hit
					//if(Ball_cx >= P1_rx){
					//	Object[i + 2].velocity.x *= -1.0f;
					//	//Push out logic
					//	overlap = std::abs(std::abs(Ball_lx) - std::abs(P1_rx));
					//	Object[i+2].position.x += overlap*P2_Speed*elapsed;
					//}
					//// Left Hit
					//if(Ball_cx <= P1_lx) {
					//	Object[i + 2].velocity.x *= -1.0f;
					//	//Push out logic
					//	overlap = std::abs(std::abs(Ball_rx) - std::abs(P1_lx));
					//	Object[i+2].position.x -= overlap*P2_Speed*elapsed;
					//}					

					if(i + 2 < 12) // P1 balls hitting P2 Paddle
						swap_ball(&Object[i+2], 1); // Swap to P2 Ball

				}
			}
		}


		// ==============================
		// Ball Brick Collision
		// ==============================
		int brick_dir = 0;
		for(uint8_t i = 2; i < 22; i++) {
			if(Object[i].position.y != -500.0f && Object[i].position.x != -500.0f) { // Balls in Play Area
				for(uint32_t j = 22; j < 190; j++) { //Bricks
					
					if(check_overlap(&Object[j], brick_dim, &Object[i], ball_dim))
					{
						if(i < 12) {
							P1_score += 1;
							brick_dir = 1;
						}
						else {
							P2_score += 1;
							brick_dir = 2;
						}
						randnum = std::rand() % 10;
						
						if(randnum == 1) // 10% change of power up 
						{
							for(uint32_t k = 0; k < 20; k++){
								if(Object[k+192].position.x == -500.0f && Object[i+192].position.y == -500.0f) {
									Object[k + 192].position = Object[j].position;
									all_brick_dir[k] = brick_dir;
									break;
								}
							}
						}
						float Brick_lx = Object[j].position.x - brick_dim.x/2.0f;
						float Brick_rx = Object[j].position.x + brick_dim.x/2.0f;
						//float Ball_lx =  Object[i].position.x - ball_dim.x/2.0f;
						//float Ball_rx =  Object[i].position.x + ball_dim.x/2.0f;
						float Ball_cx =  Object[i].position.x;
						//float Ball_cx =	 Object[i].position.x;
						hide_object(&Object[j]);
						if(Ball_cx <= Brick_lx) // Left Collision
							Object[i].velocity.x *= -1.0f;
						else if(Ball_cx >= Brick_rx) // Right Collision
							Object[i].velocity.x *= -1.0f;
						else // Top/Bottom Collision
							Object[i].velocity.y *= -1.0f;
						

					}
				}
			}
		}

		// ===========================
		// Score Update
		// ===========================
		
		Object[26].name = std::string("P2 score:") + std::to_string(P2_score);
		Object[25].name = std::string("P1 score:") + std::to_string(P1_score);

		// ============================
		// Power Up Movement 
		// ============================
		for(uint32_t i = 0; i < 20; i++){
			if(all_brick_dir[i] == 1) {
				Object[i + 192].position.y -= powerup_speed*elapsed;
				if(Object[i + 192].position.y <= vertical_bounds.x) {
					hide_object(&Object[i + 192]);
					all_brick_dir[i] = 0;
					
				}
			}
			else if(all_brick_dir[i] == 2) {
				Object[i + 192].position.y += powerup_speed*elapsed;
				if(Object[i + 192].position.y >= vertical_bounds.y) {
					hide_object(&Object[i + 192]);
					all_brick_dir[i] = 0;
				}
			}
		}

		// ==============================
		// Power Up Collision
		// ==============================
		
		for(uint32_t i = 0; i < 20; i++) {
			// P1 Check
			if(check_overlap(&Object[P1_id], paddle_dim_P1, &Object[i+192], powerup_dim)) {
				hide_object(&Object[i+192]);
				//printf("P1 Power Up!\n");// Enable Power Up
				randnum = rand() % 5;
				P1_powerup_bitmap[randnum] = 1;
				all_brick_dir[i] = 0;
			}
			// P2 Check 
			if(check_overlap(&Object[P2_id], paddle_dim_P2, &Object[i+192], powerup_dim)) {
				hide_object(&Object[i+192]);
				//printf("P2 Power Up!\n");// Enable Power Up
				randnum = rand() % 5;
				P2_powerup_bitmap[randnum] = 1;
				all_brick_dir[i] = 0;
			}
		}

		// ===============================
		// Power Up Enable
		// ===============================
		// 0 - Long, 1 - Reverse, 2 - Multi, 3 - Make color, 4 - Faster Speed 
		// ---------------P1---------------
		if(P1_powerup_bitmap[0] == 1 && P1_long == 0) { // Long
			P1_long = 1;
			Object[190].position = Object[0].position;
			hide_object(&Object[0]);
			P1_powerup_bitmap[0] = 0;
		}

		if(P1_powerup_bitmap[1] == 1) // Reverse
		{	
			Object[209].name = "P1 Reverse Dir: Press W";
			if(Object[0].controls.up.pressed == 1) {
				for(uint8_t i = 0; i < 20; i++){ // Reverse all balls in Play Area
					if(Object[i + 2].position.x != -500.0f) {
						Object[i + 2].velocity *= -1.0f;
					}
				}
				Object[209].name = "";
				P1_powerup_bitmap[1] = 0;

			}
		}
		
		play_area_balls = 0;

		if(P1_powerup_bitmap[2] == 1) // Spawn multi P1 Balls
		{	
			
			for(uint8_t i = 0; i < 20; i++){
				if(Object[i + 2].position.x != -500.0f) {
					if(i + 2 < 12) { // How many balls in PlayArea are P1
						play_area_balls += 1;
					}
				}
			}

			play_area_balls = 5 - play_area_balls; // Max spawn 5

			for(uint8_t i = 0; i < 20; i++){ // Spawn
				if(Object[i + 2].position.x == -500.0f && play_area_balls > 0) {
					if(i + 2 < 12) { // Spawn Balls
						randnum = 1 + std::rand() % 10;
						float x = randnum/10.0f;
						float y = 1.0f - x;
						if(std::rand()%2 == 0)
							Object[i + 2].velocity.x = x*ball_speed; // Set initial velocity
						else 
							Object[i + 2].velocity.x = -1.0f*x*ball_speed; // Set initial velocity

						Object[i + 2].velocity.y = y*ball_speed; // Set initial velocity
						Object[i + 2].position = P1_Ball_Spawn_Loc; // Set position
						play_area_balls -= 1;
					}
				}
			}

			P1_powerup_bitmap[2] = 0;
		}


		if(P1_powerup_bitmap[3] == 1) // Color Set all Ball
		{	
			Object[208].name = "P1 Switch Color: Press Space";
			if(Object[0].controls.space.pressed == 1) {
				for(uint8_t i = 0; i < 20; i++){ // Set all balls in Play Area to P1 Balls
					if(Object[i + 2].position.x != -500.0f) {
						if(i + 2 >= 12) // Current block is P2 Ball 
						{
							swap_ball(&Object[i + 2], 0);
						}  
					}
				}
				Object[208].name = "";
				P1_powerup_bitmap[3] = 0;
			}
		}


		if(P1_powerup_bitmap[4] == 1 && P1_fast == 0) { // Fast
			P1_fast = 1;
			P1_powerup_bitmap[4] = 0;
			P1_Speed *= 2.0f;
		}


		// ---------------P2---------------
		if(P2_powerup_bitmap[0] == 1 && P2_long == 0) { // Long
			P2_long = 1;
			Object[191].position = Object[1].position;
			hide_object(&Object[1]);
			P2_powerup_bitmap[0] = 0;
		}

		if(P2_powerup_bitmap[1] == 1) // Reverse
		{	
			Object[211].name = "P2 Reverse Dir: Press W";
			if(Object[1].controls.up.pressed == 1) {
				for(uint8_t i = 0; i < 20; i++){ // Reverse all balls in Play Area
					if(Object[i + 2].position.x != -500.0f) {
						Object[i + 2].velocity *= -1.0f;
					}
				}
				Object[211].name = "";
				P2_powerup_bitmap[1] = 0;

			}
		}

		
		play_area_balls = 0;

		if(P2_powerup_bitmap[2] == 1) // Spawn multi P2 Balls
		{	
			
			for(uint8_t i = 0; i < 20; i++){
				if(Object[i + 2].position.x != -500.0f) {
					if(i + 2 >= 12) { // How many balls in PlayArea are P2
						play_area_balls += 1;
					}
				}
			}

			play_area_balls = 5 - play_area_balls; // Max spawn 5

			for(uint8_t i = 0; i < 20; i++){ // Spawn
				if(Object[i + 2].position.x == -500.0f && play_area_balls > 0) {
					if(i + 2 >= 12) { // Spawn Balls
						randnum = 1 + std::rand() % 10;
						float x = randnum/10.0f;
						float y = 1.0f - x;
						if(std::rand()%2 == 0)
							Object[i + 2].velocity.x = x*ball_speed; // Set initial velocity
						else 
							Object[i + 2].velocity.x = -1.0f*x*ball_speed; // Set initial velocity

						Object[i + 2].velocity.y = -y*ball_speed; // Set initial velocity
						Object[i + 2].position = P2_Ball_Spawn_Loc; // Set position
						play_area_balls -= 1;
					}
				}
			}

			P2_powerup_bitmap[2] = 0;
		}


		
		if(P2_powerup_bitmap[3] == 1) // Color Set all Ball
		{	
			Object[210].name = "P2 Switch Color: Press Space";
			if(Object[1].controls.space.pressed == 1) {
				for(uint8_t i = 0; i < 20; i++){ // Set all balls in Play Area to P2 Balls
					if(Object[i + 2].position.x != -500.0f) {
						if(i + 2 < 12) // Current block is P1 Ball 
						{
							swap_ball(&Object[i + 2], 1);
						}  
					}
				}
				Object[210].name = "";
				P2_powerup_bitmap[3] = 0;
			}
		}

		if(P2_powerup_bitmap[4] == 1 && P2_fast == 0) { // Fast
			P2_fast = 1;
			P2_powerup_bitmap[4] = 0;
			P2_Speed *= 2.0f;
		}


		// ===============================
		// Game Over Logic
		// ===============================
		int game_over = 1;
		for(uint32_t i = 0; i < 20; i++) {
			if(Object[i + 2].position.x != -500.0f) { // Some balls still in play area
				game_over = 0;
				break;
			}
		}
		if(game_over == 1){
				init(1);
		}
	}
	
}


void Game::send_state_message(Connection *connection_, Player *connection_player) const {
	assert(connection_);
	auto &connection = *connection_;

	connection.send(Message::S2C_State);
	//will patch message size in later, for now placeholder bytes:
	connection.send(uint8_t(0));
	connection.send(uint8_t(0));
	connection.send(uint8_t(0));
	size_t mark = connection.send_buffer.size(); //keep track of this position in the buffer


	//send player info helper:
	auto send_player = [&](Player const &player) {
		connection.send(player.position);
		connection.send(player.velocity);
		connection.send(player.color);
	
		//NOTE: can't just 'send(name)' because player.name is not plain-old-data type.
		//effectively: truncates player name to 255 chars
		uint8_t len = uint8_t(std::min< size_t >(255, player.name.size()));
		connection.send(len);
		connection.send_buffer.insert(connection.send_buffer.end(), player.name.begin(), player.name.begin() + len);
	};

	//player count:
	//connection.send(uint8_t(players.size()));
	//if (connection_player) send_player(*connection_player);
	//for (auto const &player : players) {
	//	if (&player == connection_player) continue;
	//	send_player(player);
	//}

	// For 2 players
	connection.send(uint8_t(212));
	if (connection_player) send_player(*connection_player);
	for (auto const &player : Object) {
		if (&player == connection_player) continue;
		send_player(player);
	}

	//compute the message size and patch into the message header:
	uint32_t size = uint32_t(connection.send_buffer.size() - mark);
	connection.send_buffer[mark-3] = uint8_t(size);
	connection.send_buffer[mark-2] = uint8_t(size >> 8);
	connection.send_buffer[mark-1] = uint8_t(size >> 16);
}

bool Game::recv_state_message(Connection *connection_) {
	assert(connection_);
	auto &connection = *connection_;
	auto &recv_buffer = connection.recv_buffer;

	if (recv_buffer.size() < 4) return false;
	if (recv_buffer[0] != uint8_t(Message::S2C_State)) return false;
	uint32_t size = (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	uint32_t at = 0;
	//expecting complete message:
	if (recv_buffer.size() < 4 + size) return false;

	//copy bytes from buffer and advance position:
	auto read = [&](auto *val) {
		if (at + sizeof(*val) > size) {
			printf("Size = %zd %d",  at + sizeof(*val), size);
			throw std::runtime_error("Ran out of bytes reading state message.");
		}
		std::memcpy(val, &recv_buffer[4 + at], sizeof(*val));
		at += sizeof(*val);
	};

	players.clear();
	uint8_t player_count;
	read(&player_count);
	// ==================
	// For all Players 
	// ==================
	 for (uint8_t i = 0; i < player_count; ++i) {
		//players.emplace_back();
		//Player &player = players.back();
		read(&Object[i].position);
		read(&Object[i].velocity);
		read(&Object[i].color);
		uint8_t name_len;
		read(&name_len);
		//n.b. would probably be more efficient to directly copy from recv_buffer, but I think this is clearer:
		Object[i].name = "";
		for (uint8_t n = 0; n < name_len; ++n) {
			char c;
			read(&c);
			Object[i].name += c;
		}
	}

	//for (uint8_t i = 0; i < player_count; ++i) {
	//	players.emplace_back();
	//	Player &player = players.back();
	//	read(&player.position);
	//	read(&player.velocity);
	//	read(&player.color);
	//	uint8_t name_len;
	//	read(&name_len);
	//	//n.b. would probably be more efficient to directly copy from recv_buffer, but I think this is clearer:
	//	player.name = "";
	//	for (uint8_t n = 0; n < name_len; ++n) {
	//		char c;
	//		read(&c);
	//		player.name += c;
	//	}
	//}

	if (at != size) throw std::runtime_error("Trailing data in state message.");

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + size);

	return true;
}
