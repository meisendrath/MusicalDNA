#include "LElement.h"

#include "HAL.h"
#include "TAtlas.h"
#include "GameStateDefines.h"
#include "FPGameStatus.h"
#include "GameState.h"
#include "TMessage.h"

#include "math.h"


extern int gGameState;
extern int gScreenWidth, gScreenHeight;
extern float gScreenAspectRatio;


extern double curtimeval;


extern TAtlas *hud_g_atlas;

extern float gHUD_color[4];

extern HAL *gHAL;
extern FPGameStatus gGS;


extern double getcurrenttime();



//
std::vector<TMessage>	gMessageQueue;			// SState or LElement messages queued until just before next update

std::vector<TActiveTouch*> TActiveTouch::active_touches;


LElement::LElement() { extent[0] = extent[1] = 0; extent[2] = gScreenWidth; extent[3] = gScreenHeight; gElements.add(this); visible = true; }
void LElement::send(const char *msg, float arg, float arg2) {
	TMessage m;
	m.msg = msg;
	m.arg1 = arg;
	m.arg2 = arg2;
	gMessageQueue.push_back(m);

}
void LElement::send_NO_QUEUE(const char *msg, float arg, float arg2) {
	GameState *state_ptr = GameState::gStates.get(gGameState);
	TMessage m;
	m.msg  = msg;
	m.arg1 = arg;
	m.arg2 = arg2;
	state_ptr->deliver_message(m);
}
void LElement::send(const TMessage &msg) {
	gMessageQueue.push_back(msg);

}
void LElement::send_NO_QUEUE(const TMessage &msg) {
	GameState *state_ptr = GameState::gStates.get(gGameState);
	state_ptr->deliver_message(msg);
}

void LElement::render_overlay(double now) {
}

void LElement::change_state(const char *new_state_name, float arg) { TMessage m; m.to = "state_change"; m.msg = new_state_name; m.arg1 = arg; gMessageQueue.push_back(m); }

void LElement::setup_center_square() {
	visible = true;
	float dim1 = fminf(gScreenWidth, gScreenHeight);
	float dim2 = fmaxf(gScreenWidth, gScreenHeight);
	if (gScreenWidth < gScreenHeight) {
		extent[0] = 0;
		extent[1] = (dim2 - dim1) / 2;
	} else {
		extent[0] = (dim2 - dim1) / 2;
		extent[1] = 0;
	}
	extent[2] = dim1;
	extent[3] = dim1;
}

LMList<LElement> LElement::gElements;
LMList<LElement> LElement::gScene;


