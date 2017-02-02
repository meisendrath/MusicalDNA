#include "Application.h"

#include "HAL.h"
#include "TAtlas.h"

#include <stdio.h>

#include "XMLParser.h"
#include "FPGameStatus.h"

#include "Skybox.h"
#include "Stars.h"

#include "TMessage.h"
#include "FreePlayDispatcher.h"
#include "FP_ROUTING.h"

#include "math.h"




//int bookmark;
double app_start_time;
double last_frame_time;

double curtimeval = 0;
double curtimeval_p = 0;


extern HAL *gHAL;


int gScreenWidth, gScreenHeight;



extern FPGameStatus gGS;

extern bool gamestate_data_file_exists;
extern bool gGamePaused;
extern int gGameState;
extern bool gamestate_data_has_changed;			// this is only set if there has been a mission start since the last new game or restore game
extern bool gamestate_data_in_memory_valid;	// either a state xml file has been read or the player chose New Game, so the saved state xml file is now out-of-date
extern int g_server_state;
extern std::string g_server_message;
extern std::string g_server_message_detail;





bool shellInitApplication()
{
	stars_init();

	app_start_time = curtimeval;
	last_frame_time = app_start_time;

	init_skybox();

	return true;
}

bool shellQuitApplication(bool full_cleanup)
{
	stars_exit();		//+BAS

	return true;
}

double wall_time;

bool shellUpdateScene(double t)
{
	bool retval = true;
	wall_time = t;
//reset_profiled_code();

	double now = curtimeval_p;
	float frame_time = (float)(now - last_frame_time);
	last_frame_time = now;
	if (frame_time < 0) frame_time = 0;
	if (frame_time > 0.1f) frame_time = 0.1f;

//	if (!HUD_game_enabled()) frame_time = 0;
    TMessage::update_send_messages();

	game_state_dispatcher(gGameState, GAMESTATE_MESSAGE_UPDATE, curtimeval, frame_time);

	return retval;
}



bool shellRenderScene() {
	gHAL->new_frame_start();
	game_state_dispatcher(gGameState, GAMESTATE_MESSAGE_RENDER, curtimeval);


#ifdef SHOW_DEBUG
	{
		double now = curtimeval;
		std::string statename;
		game_state_dispatcher(gGameState, GAMESTATE_MESSAGE_NAME, now, 0, 0, &statename);
		std::string s;
		s = format("%s: ", statename.c_str());
		float scl = fminf(gScreenWidth, gScreenHeight) / 768.0f;
		draw_stringC(TFONT_12, s.c_str(), 11*scl, 21*scl, 0, 0, 0, 1);
		draw_stringC(TFONT_12, s.c_str(), 10*scl, 20*scl, 1, 1, 1, 1);
	}
#endif // SHOW_DEBUG

//	gHAL->matrix_pop();
	gHAL->new_frame_end();

	return true;
}



