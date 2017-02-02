#include "GameState.h"

#include "GameStateDefines.h"
#include "FreePlayDispatcher.h"
#include "HAL.h"
#include "FPGameStatus.h"
#include "math.h"
#include "FreePlayVis.h"
#include "FP_SYNTH.h"
#include "FP_VISUALIZER.h"
#include "FP_ROUTING.h"
#include "skybox.h"
#include "stars.h"




// this is disabled in a different way, it was not working well
#define DIM_OUT_BACKGROUND_BEHIND_OPTIONS


#define M_2PI			(3.1415926535897932384626433 * 2.0)

#define ADJUST_FOR_SMALLER_TOP_BOT_BARS (9*scl)



extern HAL *gHAL;
FPGameStatus gGS;
extern int gScreenWidth, gScreenHeight;
extern float gScreenAspectRatio;
extern bool gGamePaused;
extern int gGameState;
extern int gGameState_pending;
extern int gGameState_prev;
extern int gGameState_pending_prev;
extern double app_start_time;







#define TITLE_SCREEN_HOLD_TIME 0.05f

extern std::string g_server_message;
extern std::string g_server_message_detail;

extern double curtimeval;
extern double curtimeval_p;


extern bool shellInitApplication();
extern double getcurrenttime();


// MARK: ----------------------------------- STATE SUPERCLASS -----------------------------------

extern std::vector<TMessage>	gMessageQueue;			// SState or LElement messages queued until just before next update

void GameState::send(const char *msg, float arg) { TMessage m; m.msg = msg; m.arg1 = arg; gMessageQueue.push_back(m); }
void GameState::send2(const char *msg, float arg1, float arg2) { TMessage m; m.msg = msg; m.arg1 = arg1; m.arg2 = arg2; gMessageQueue.push_back(m); }
void GameState::sendstr(const char *msg, const std::string &arg, float arg1) { TMessage m; m.msg = msg; m.arg0 = arg; m.arg1 = arg1; gMessageQueue.push_back(m); }
void GameState::deliver_message(const TMessage &msg) {
	bool handled = message(msg);
	for (int i=0; i<LElement::gScene.size(); i++) if (LElement::gScene[i]->visible) {
	if (!handled)
		handled |= LElement::gScene[i]->message(msg);
		if (handled) break;
	}
if (!handled) printf("!!! WARNING: message not handled: '%s' %g %g\n", msg.msg.c_str(), msg.arg1, msg.arg2);
}
void GameState::change_state(const char *new_state_name, float arg) { TMessage m; m.to = "state_change"; m.msg = new_state_name; m.arg1 = arg; gMessageQueue.push_back(m); }
void GameState::change_state_back(float arg) { TMessage m; m.to = "state_change_back"; m.arg1 = arg; gMessageQueue.push_back(m); }
void GameState::imm_message_to_state(const char *state_name, const char *msg, float arg) { GameState *s = GameState::gStates.get(state_name); if (s != NULL) s->message(msg, arg, 0); }
void GameState::read_psd(bool ignore_aspect_ratio, const char *psdname) {
    if (psdname == NULL) psdname = name.c_str();
    if (psd_file_name.compare(psdname) != 0) {
        delete psd_file;
        psd_file = NULL;
        psd_file_name = psdname;
    }
}
GameState *GameState::get_state_by_ident(int findident) {
	GameState *retval = NULL;
	for (int i=0; i<gStates.size(); i++) if (findident == gStates[i]->ident) { retval = gStates[i]; break; }
	return retval;
}
LMList<GameState> GameState::gStates;


// MARK: ----------------------------------- LOADING -----------------------------------




class SLoadingState : public GameState {
public:
    SLoadingState() { name = "LOADING_SCREEN"; ident = GAMESTATE_LOADING_SCREEN; startup_state = 0; did_loading = false; }
    virtual						~SLoadingState() { }
public:
    int							startup_state;
    bool						did_loading;
public:
    virtual void				update(double now, float frame_time) {
        if ((startup_state == 1) && ((curtimeval - gGS.gGameState_start_time) >
                                     TITLE_SCREEN_HOLD_TIME
                                     )) {
            if (!shellInitApplication())
                printf("InitApplication error\n");
            
            
            change_state("FREE_PLAY");
            startup_state = 0;
        }
    }
    virtual void				render(double now) {
        gHAL->render_bg_setup();
        gHAL->set_mode(0);
        gHAL->set_mode(5);		// clear the depth buffer
        gHAL->set_culling(false, false);
        gHAL->dfill_rect(0, 0, gScreenWidth, gScreenHeight, 0, 0, 0, 0, 1);
        startup_state = 1;
    }
    virtual void				exit(double now) {
    }
    virtual void				enter(double now, float arg1) {
        if (!did_loading) {
            did_loading = true;
        }
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MARK: ----------------------------------- SERVER MESSAGE -----------------------------------

#define SERVER_MESSAGE_DONE (server_startup_done())


class CFBackground_Atlas_Mixin {		// mixin to get mdna_atlas and mdna2_atlas
    //								CFBackground_Atlas_Mixin() { }
public:
    static TAtlas				*mdna_atlas;
    static TAtlas				*mdna2_atlas;
public:
    virtual void render_callback(int ref) {
        float scl = fminf(gScreenWidth, gScreenHeight) / 768.0f;
        int fb3 = mdna2_atlas->get_atlas_frame_number_for_file_name("cmenubar.png");
        float ascl3 = 518.0f / mdna2_atlas->get_atlas_frame_height(fb3);
        
        gHAL->matrix_push();
        gHAL->matrix_translate(gScreenWidth/2, 0, 0);
        gHAL->matrix_scale(scl * ascl3, scl * ascl3, 1);
        mdna2_atlas->draw_atlas_texture_c(fb3, 0, mdna2_atlas->get_atlas_frame_height(fb3)/2);
        gHAL->matrix_pop();
        
        gHAL->matrix_push();
        gHAL->matrix_translate(gScreenWidth/2, gScreenHeight, 0);
        gHAL->matrix_rotate(180, 0, 0, 1);
        gHAL->matrix_scale(scl * ascl3, scl * ascl3, 1);
        mdna2_atlas->draw_atlas_texture_c(fb3, 0, mdna2_atlas->get_atlas_frame_height(fb3)/2);
        gHAL->matrix_pop();
    }
    
    static void HUD_draw_background(TMatrix *camera_matrix = NULL) {
        if (mdna2_atlas == NULL) mdna2_atlas = TAtlas::get_cached_or_load("MDNA2");
        gHAL->dfill_rect(0, 0, gScreenWidth, gScreenHeight, 0, 0.0f, 0.0f, 0.0f, 0.0f);
        TVector3 v(0, 0, 0);
        
        gHAL->matrix_push();
        gHAL->matrix_push(1);
        gHAL->matrix_push(2);
        float fov = 0.785398185f;
        TMatrix proj;
        TMatrix::perspectiveFovRH(proj, fov, gScreenAspectRatio, 4.0f, 50000.0f, false);
        gHAL->matrix_load(proj, 1);
        gHAL->matrix_loadidentity();
        gHAL->matrix_loadidentity(2);
        
        if (camera_matrix != NULL) {
            gHAL->matrix_multiply(*camera_matrix, 2);
        }
        //
        //#warning set up projection frustum
        //
        float t = (float)curtimeval;
        t += 20000;
        gHAL->matrix_rotate((float)(t * 0.220f), 0, 0, 1, 2);
        
        
            DrawSkybox(1.0f, v, true);
        
        float speed = 100.0f;
        TMatrix m;
        TMatrix::identity(m);
        m.f[12] = v.x;
        m.f[13] = v.y;
        m.f[14] = (float)(v.z - curtimeval * speed);
        TVector3 v2(0, 0, (float)(curtimeval * speed));
        gHAL->matrix_translate(v2.x, v2.y, v2.z, 2);
        
        stars_draw(&m, 1.0f);			// this does update & render
        
        gHAL->matrix_pop(2);
        gHAL->matrix_pop(1);
        gHAL->matrix_pop();
        //#warning back to ortho
        
        if (mdna2_atlas != NULL) {
            float scl = fminf(gScreenWidth, gScreenHeight) / 768.0f;
            int fb2 = mdna2_atlas->get_atlas_frame_number_for_file_name("cmenubarbg.png");
            float ascl = 518.0f / mdna2_atlas->get_atlas_frame_height(fb2);
            
            gHAL->matrix_push();
            gHAL->matrix_translate(gScreenWidth/2, 0, 0);
            gHAL->matrix_scale(scl * ascl, scl * ascl, 1);
            mdna2_atlas->draw_atlas_texture_c(fb2, 0, mdna2_atlas->get_atlas_frame_height(fb2)/2);
            gHAL->matrix_pop();
            
            gHAL->matrix_push();
            gHAL->matrix_translate(gScreenWidth/2, gScreenHeight, 0);
            gHAL->matrix_rotate(180, 0, 0, 1);
            gHAL->matrix_scale(scl * ascl, scl * ascl, 1);
            mdna2_atlas->draw_atlas_texture_c(fb2, 0, mdna2_atlas->get_atlas_frame_height(fb2)/2);
            gHAL->matrix_pop();
            
        }
        
    }
    
    static void HUD_draw_background_overlay(const char *gamename="freeplay.png")
    {
        if (mdna2_atlas == NULL) mdna2_atlas = TAtlas::get_cached_or_load("MDNA2");
        float scl = fminf(gScreenWidth, gScreenHeight) / 768.0f;
        if (mdna2_atlas != NULL) {
            
            int fc2 = mdna2_atlas->get_atlas_frame_number_for_file_name(gamename);
            float ascl = 233.0f / mdna2_atlas->get_atlas_frame_height(fc2);
            //printf("HUGE ATLAS %2d: %dx%d AUTO-SIZED %s\n", fc2, mdna2_atlas->get_atlas_frame_width(fc2), mdna2_atlas->get_atlas_frame_height(fc2), mdna2_atlas->get_atlas_original_file_name(fc2));
            
            gHAL->matrix_push();
            gHAL->matrix_translate(gScreenWidth/2, 0, 0);
            gHAL->matrix_scale(scl / 2.1333f, scl / 2.1333f, 1);
            gHAL->matrix_scale(ascl, ascl, 1);
            gHAL->matrix_translate(0, mdna2_atlas->get_atlas_frame_height(fc2)/2 - 4*scl, 0);
            mdna2_atlas->draw_atlas_texture_c(fc2, 0, 0);
            gHAL->matrix_pop();
        }
    }
};
TAtlas				*CFBackground_Atlas_Mixin::mdna_atlas = NULL;
TAtlas				*CFBackground_Atlas_Mixin::mdna2_atlas = NULL;


void GameState::render_callback(int ref) {
	CFBackground_Atlas_Mixin *s = dynamic_cast<CFBackground_Atlas_Mixin *>(this);
	if (s != NULL) s->render_callback(ref);
}



// MARK: ----------------------------------- CENTERFIRE: ROUTING -----------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void				setup_initial_routing() {	// "normal routing, CF_SEARCH and BL_SEARCH isolated"
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_SYNTH, RB_NONE);
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_SYNTH_DRUMS, RB_NONE);
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_SYNTH_VIZ_VOICE, RB_ALL);
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_RECORD_NOTE, RB_ALL);
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_VIZUALIZER, RB_ALL);
	ROUTING_set_route(ROUTING_FROM_TOUCH_INPUT, ROUTING_TO_MIDI_OUT, RB_ALL);

}

static void				stop_playing() {
	setup_initial_routing();
}

static void				bail_to_main_menu() {
	ROUTING_all_notes_off(ROUTING_FROM_APPLICATION);
	FP_VISUALIZER_recent_notes_off();
	FP_VISUALIZER_allgviznotesoff();
	stop_playing();
}


class FreePlayState : public GameState, public CFBackground_Atlas_Mixin {
public:
								FreePlayState() { name = "FREE_PLAY"; ident = GAMESTATE_FREE_PLAY; substate = 0; }
	virtual						~FreePlayState() { }
public:
	int							substate;
public:
	virtual void				update(double now, float frame_time) {
		if (substate == 1) {
			FP_SYNTH_set_patch(128, 1, true);
			substate = 2;
		}
		if (substate == 0) {
			substate = 1;
		}
		//
	}
	virtual void				render(double now) {		
		HUD_draw_background();
	}
	virtual void				render_overlay(double now) {			// overlays do most of their rendering here, after the elements list LElement::gScene[] is drawn
		HUD_draw_background_overlay();
	}
	virtual void				enter(double now, float arg1) {
		FP_VISUALIZER_init();
		init_freeplay();

		if (mdna_atlas == NULL) mdna_atlas = TAtlas::get_cached_or_load("MDNA");

		LElement *img;
		LE_GET_OR_CREATE(img, "CENTER_Visualizer", new FreePlayVis(), { /* one-time stuff, not screensize related */ });
		img->setup_center_square();
		LElement::gScene.add(img);

		if (arg1 == 0) substate = 0;

	}
	virtual void				exit(double now) {
		bail_to_main_menu();
	}
	virtual bool				message(const std::string &msg, float arg1, float arg2) {
		bool retval = false;
        printf("FreePlayState MESSAGE IGNORED: %s\n", msg.c_str());
		return retval;
	}
};



void GameState::free_play_dispatcher_create_states() {
	if (GameState::gStates.size() > 0) return;		// one-time call

	GameState::gStates.add(new SLoadingState());
	GameState::gStates.add(new FreePlayState());
}

