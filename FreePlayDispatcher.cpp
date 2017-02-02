#include "FreePlayDispatcher.h"

#include "GameStateDefines.h"
#include "HAL.h"
#include "TAtlas.h"
#include "FPGameStatus.h"
#include "LElement.h"
#include "GameState.h"


// dup from common.h: (NOT SURE WHY THIS IS NEEDED!!!)
#ifdef __APPLE__
extern void logprintf(const char* fmt, ...);
#define printf(fmt, args...)  logprintf(fmt, ##args)
#endif // __APPLE__

int gGameState = GAMESTATE_STARTUP_STATE;					// first state	//STATE
int gGameState_pending = GAMESTATE_STARTUP_STATE;			// state from xml file
int gGameState_pending_prev = GAMESTATE_STARTUP_STATE;		// state from xml file
int gGameState_prev, gGameState_prev_prev, gGameState_prev_prev_prev;
GameState *gCurState_ptr = NULL;

int new_game_state_transition_type = 0;
double new_game_state_transition_start_time = 0;			// the value of now when the transition started
float new_game_state_transition_time = 1.0f;				// how long the transition takes



// stuff used here, declared elsewhere:
extern double curtimeval;
extern HAL *gHAL;
extern FPGameStatus gGS;
extern int gScreenWidth, gScreenHeight;
extern float gDeviceScale;

extern std::vector<TMessage>	gMessageQueue;			// SState or LElement messages queued until just before next update



// stuff not used here, but referenced elsewhere as extern:

extern void report_memory();

extern void renderDebugStrings();

extern void play_sound(int which);

static float free_play_dispatcher_2(int cur_state, int message, double now, float arg1=0, float arg2=0, void *argptr=NULL);	// fwd



// change to a new state
static void free_play_dispatcher_change_state(int new_game_state, int message, double now, float arg1=0) {
    new_game_state_transition_start_time = now;
    //++ append all elements on LElement::gScene to LElement::gSliding_off
	for (int scii=0; scii<LElement::gScene.size(); scii++) LElement::gScene[scii]->cleanup_on_exit();
    LElement::gScene.remove_all();		// state change - clear out gScene before GAMESTATE_MESSAGE_ENTER

    free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_EXIT, now);		// may move items from LElement::gSliding_off to LElement::gScene if they are to stay on screen

    gGS.gGameState_start_time = curtimeval;

	std::string st1;
	std::string st2;
	free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_NAME, now, 0, 0, &st1);
	free_play_dispatcher(new_game_state, GAMESTATE_MESSAGE_NAME, now, 0, 0, &st2);
	HAL::report_event("State Change", "From", st1.c_str(), "To", st2.c_str());

    gGameState = new_game_state;
    gCurState_ptr = GameState::gStates.get(new_game_state);
	for (int i=0; i<TActiveTouch::active_touches.size(); i++) delete TActiveTouch::active_touches[i];
    TActiveTouch::active_touches.resize(0);

    // recursing allows ENTER to change the state? does that mess up transitions? the issue is losing the elements on LElement::gSliding_off
    // ??? probably should try this out: have one state ENTER change to another, second state sets up transition
    free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_ENTER, now, arg1);	// may remove items from LElement::gSliding_off, may add named items to LElement::gSliding_on (w/ transition) and LElement::gScene
}

// deliver the messages (necessarily decoupled from sending)
void free_play_dispatcher_process_messages(double now) {
    //++++ grab a message, peel it off the list, send it to the current state (and allow that to change), repeat
    int st = gGameState;
    while (gMessageQueue.size() > 0) {
        TMessage msg = gMessageQueue[0];
        gMessageQueue.erase(gMessageQueue.begin());
        if (strcmp("state_change", msg.to.c_str()) == 0) {
            GameState *dest_state_ptr = GameState::gStates.get(msg.msg.c_str());
            gGameState_prev_prev_prev = gGameState_prev_prev;
            gGameState_prev_prev = gGameState_prev;
            gGameState_prev = gGameState;
            free_play_dispatcher_change_state(dest_state_ptr->ident, GAMESTATE_MESSAGE_STATE_CHANGE, now, msg.arg1);
        } else if (strcmp("state_change_back", msg.to.c_str()) == 0) {
            int state = gGameState_prev;
            gGameState_prev = gGameState_prev_prev;
            gGameState_prev_prev = gGameState_prev_prev_prev;
            free_play_dispatcher_change_state(state, GAMESTATE_MESSAGE_STATE_CHANGE, now, msg.arg1);
        } else {
            free_play_dispatcher_2(gGameState, GAMESTATE_MESSAGE_TMESSAGE, now, msg.arg1, msg.arg2, (void *)&msg);
        }

        if (gGameState != st) {
            // a detected state change flushes outstanding messages
            while (gMessageQueue.size() > 0) {
                TMessage mmsg = gMessageQueue[0];
                gMessageQueue.erase(gMessageQueue.begin());
            }
        }
    }
}


// dispatch a game message to the state and/or the list of visible elements
//// caller does this first: virtual screen size changes: send ENTER to current state again after change; modify input touch coords by 200% on retina
static float free_play_dispatcher_2(int cur_state, int message, double now, float arg1, float arg2, void *argptr) {
    if ((gGameState == GAMESTATE_STARTUP_STATE) && (message == GAMESTATE_MESSAGE_ENTER)) {		// BOOT UP SEQUENCE
        GameState::free_play_dispatcher_create_states();
    }

    if (message == GAMESTATE_MESSAGE_UPDATE) {													// MESSAGE DELIVERY
    	free_play_dispatcher_process_messages(now);
        cur_state = gGameState;			// controversial, but try this (UPDATE cannot be dispatched to any state other than gGameState)
    }

	float retval = 0.0f;
	int new_game_state = cur_state;
    if ((gCurState_ptr == NULL) || (gCurState_ptr->ident != cur_state)) {
    	gCurState_ptr = GameState::gStates.get(cur_state);
    }
	switch (message) {
        case GAMESTATE_MESSAGE_UPDATE:					// may change new_game_state

            gCurState_ptr->update(now, arg1);
            for (int i=0; i<LElement::gScene.size(); i++)
                if (LElement::gScene[i]->visible)
                    LElement::gScene[i]->update(now, arg1);
            break;
        case GAMESTATE_MESSAGE_RENDER:
            gHAL->matrix_loadidentity(1);
		    if (gGS.user_screen_flip) {
                gHAL->matrix_ortho(gScreenWidth, 0, 0, gScreenHeight, -1000.0f, 1000.0f);
            } else {
                gHAL->matrix_ortho(0, gScreenWidth, gScreenHeight, 0, -1000.0f, 1000.0f);
            }
            gHAL->matrix_loadidentity();


            gCurState_ptr->render(now);

            for (int i=0; i<LElement::gScene.size(); i++) if (LElement::gScene[i]->visible) LElement::gScene[i]->super_render(now);


            if (new_game_state_transition_start_time != 0) {			// TRANSITION IN PROGRESS
            }
            gCurState_ptr->render_overlay(now);

            break;
        case GAMESTATE_MESSAGE_TOUCH_BEGIN:
            {
                TActiveTouch *a = new TActiveTouch();
                a->disambiguator = argptr;
                a->owner = NULL;
                a->sx = a->cx = arg1;
                a->sy = a->cy = arg2;
                TActiveTouch::active_touches.push_back(a);
                //
                TActiveTouch *pa = TActiveTouch::active_touches[TActiveTouch::active_touches.size()-1];
                for (int i=LElement::gScene.size(); --i>=0; ) if (LElement::gScene[i]->visible) {
					TActiveTouch *tch = LElement::gScene[i]->test_touch(0, now, pa);
					if (tch != NULL) {
						if (pa != tch) {
							for (int ij=0; ij<TActiveTouch::active_touches.size(); ij++) if (TActiveTouch::active_touches[ij] == pa) { TActiveTouch::active_touches[ij] = tch; break; }
							delete pa;
							pa = tch;
						}
						pa->owner = LElement::gScene[i];
						break;
					}
				}
            }
            break;
        case GAMESTATE_MESSAGE_TOUCH_MOVE:
            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                TActiveTouch *pa = TActiveTouch::active_touches[i];
                if (argptr == pa->disambiguator) {
                    pa->cx = arg1;
                    pa->cy = arg2;
                    if (pa->owner != NULL) pa->owner->test_touch(1, now, pa);
                    break;
                }
            }
            break;
        case GAMESTATE_MESSAGE_TOUCH_END:				// may change new_game_state
            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                TActiveTouch *pa = TActiveTouch::active_touches[i];
                if (argptr == pa->disambiguator) {
                    pa->cx = arg1;
                    pa->cy = arg2;
                    if (pa->owner != NULL) pa->owner->test_touch(2, now, pa);
					delete pa;
	                TActiveTouch::active_touches.erase(TActiveTouch::active_touches.begin() + i);
                    break;
                }
            }
            break;
        case GAMESTATE_MESSAGE_TOUCH_CANCEL:
            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                TActiveTouch *pa = TActiveTouch::active_touches[i];
                if (argptr == pa->disambiguator) {
                    if (pa->owner != NULL) pa->owner->test_touch(3, now, pa);
					delete pa;
	                TActiveTouch::active_touches.erase(TActiveTouch::active_touches.begin() + i);
                    break;
                }
            }
            break;
        case GAMESTATE_MESSAGE_TMESSAGE:					// may change new_game_state
			{
				TMessage m = *(TMessage *)argptr;
				{
					bool handled = gCurState_ptr->message(m);
					for (int i=0; i<LElement::gScene.size(); i++) if (LElement::gScene[i]->visible) {
						if (!handled)
							handled |= LElement::gScene[i]->message(m);
						if (handled) break;
					}
if (!handled) printf("!!! WARNING: message not handled: '%s'\n", m.msg.c_str());
				}
			}
            break;
			
        case GAMESTATE_MESSAGE_MESSAGE:					// may change new_game_state
			{
				TMessage m;
				m.msg  = (char *)argptr;
				m.arg1 = arg1;
				m.arg2 = arg2;
				{
					bool handled = gCurState_ptr->message(m);
					for (int i=0; i<LElement::gScene.size(); i++) if (LElement::gScene[i]->visible) {
						if (!handled)
							handled |= LElement::gScene[i]->message(m);
						if (handled) break;
					}
if (!handled) printf("!!! WARNING: message not handled: '%s' %g %g\n", (char *)argptr, arg1, arg2);
				}
			}
            break;
        case GAMESTATE_MESSAGE_ENTER:					// may change new_game_state
        	gCurState_ptr->enter(now, arg1);	// if (arg1==0) first entry(), not from rotation or other

            break;
        case GAMESTATE_MESSAGE_REENTER:					// may change new_game_state
        	gCurState_ptr->resume_enter(now, arg1);
            break;
        case GAMESTATE_MESSAGE_EXIT:
        	gCurState_ptr->exit(now);
            break;
        case GAMESTATE_MESSAGE_PLAYER_HIT:				// HANDLE THESE WITH REAL MESSAGES!!!
            gCurState_ptr->send2("hit", arg1, arg2);
            break;
        case GAMESTATE_MESSAGE_PLAYER_FREEZE:
            gCurState_ptr->send2("freeze", arg1, arg2);
            break;
        case GAMESTATE_MESSAGE_NAME:
			*(std::string *)argptr = gCurState_ptr->name;
            break;
    }

	if (new_game_state != cur_state) {
    	free_play_dispatcher_change_state(new_game_state, message, now);
    }
	//
	return retval;
}


float free_play_dispatcher(int cur_state, int message, double now, float arg1, float arg2, void *argptr) {
	switch (message) {
		case GAMESTATE_MESSAGE_TOUCH:
		case GAMESTATE_MESSAGE_TOUCH_MOVE:
		case GAMESTATE_MESSAGE_TOUCH_END:
		case GAMESTATE_MESSAGE_TOUCH_CANCEL:
		{
			float scl = gDeviceScale;
			arg1 *= scl;
			arg2 *= scl;
		}
	}
    if (gGS.user_screen_flip) {
        switch (message) {
            case GAMESTATE_MESSAGE_TOUCH:
            case GAMESTATE_MESSAGE_TOUCH_MOVE:
            case GAMESTATE_MESSAGE_TOUCH_END:
            case GAMESTATE_MESSAGE_TOUCH_CANCEL:
                arg1 = gScreenWidth - arg1;
                arg2 = gScreenHeight - arg2;
                break;
        }
    }
	return free_play_dispatcher_2(cur_state, message, now, arg1, arg2, argptr);
}

float game_state_dispatcher(int cur_state, int message, double now, float arg1, float arg2, void *argptr) {
    return free_play_dispatcher(cur_state, message, now, arg1, arg2, argptr);
}
