//
//  TMessage.cpp
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#include "TMessage.h"

#include "FreePlayDispatcher.h" // for game_state_dispatcher() and GAMESTATE_MESSAGE_MESSAGE

#include <stdlib.h>

extern int gGameState;      // for game_state_dispatcher()
extern double getcurrenttime();

void TMessage::send_message(TMessage &msg) {		// intended to be called at update time (and maybe just before saving the state so no messages need be stored)
#ifdef SHOW_MESSAGE_DETAIL
    printf("TMessage::send_message() %s: %s %s %g,%g 0x%016llx\n", msg.msg.c_str(), msg.submsg.c_str(), msg.arg0.c_str(), msg.arg1, msg.arg2, (long)(void *)msg.target);
#endif // SHOW_MESSAGE_DETAIL
        if (msg.to.compare("game") == 0) {
            // tell game_state_dispatcher
            game_state_dispatcher(gGameState, GAMESTATE_MESSAGE_TMESSAGE, getcurrenttime(), 0, 0, (void *)&msg);
        }
}

std::vector<TMessage> TMessage::pending_messages;

