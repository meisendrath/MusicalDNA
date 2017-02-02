//
//  FreePlayDispatcher.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _FREEPLAYDISPATCHER_H_
#define _FREEPLAYDISPATCHER_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>


#define GAMESTATE_MESSAGE_UPDATE 0
#define GAMESTATE_MESSAGE_RENDER 1
#define GAMESTATE_MESSAGE_TOUCH 2
#define GAMESTATE_MESSAGE_TOUCH_BEGIN 2
#define GAMESTATE_MESSAGE_TOUCH_MOVE 3
#define GAMESTATE_MESSAGE_TOUCH_END 4
#define GAMESTATE_MESSAGE_TOUCH_CANCEL 5
#define GAMESTATE_MESSAGE_ENTER 6
#define GAMESTATE_MESSAGE_REENTER 7
#define GAMESTATE_MESSAGE_EXIT 8
#define GAMESTATE_MESSAGE_PLAYER_HIT 9
#define GAMESTATE_MESSAGE_PLAYER_FREEZE 10
#define GAMESTATE_MESSAGE_NAME 11
#define GAMESTATE_MESSAGE_SPARE 12
#define GAMESTATE_MESSAGE_MESSAGE 13
#define GAMESTATE_MESSAGE_TMESSAGE 14
#define GAMESTATE_MESSAGE_STATE_CHANGE 15


float free_play_dispatcher(int cur_state, int message, double now, float arg1=0, float arg2=0, void *argptr=0L);
void free_play_dispatcher_process_messages(double now);
float game_state_dispatcher(int cur_state, int message, double now, float arg1=0, float arg2=0, void *argptr=0L);


#endif // !_FREEPLAYDISPATCHER_H_
