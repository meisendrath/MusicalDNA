//
//  TMessage.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _TMESSAGE_H
#define _TMESSAGE_H

#include "config.h"
#include "common.h"

#include <vector>
#include <string>

//#include "XMLParser.h"



//
#define SHOW_MESSAGE_DETAIL

// a saved message, to be sent just before update time
//        	<testmessage to="mbox" arg1="9" arg0="gt" arg2="3"/>			// check a mailbox to see if this chunk SHOULD BE spawned
//        	<testmessage to="" msg="" submsg="" arg0="" arg1="" arg2=""/>	// send this message TO ASK if this chunk SHOULD BE spawned
//        	<message to="mbox" arg1="9" arg0="add" arg2="3"/>				// change a mailbox AFTER this chunk is spawned
//        	<message to="" msg="" submsg="" arg0="" arg1="" arg2=""/>		// send this message AFTER this chunk is spawned
class TMessage  {
public:
						TMessage() {
                            arg1 = arg2 = 0;
                            target = NULL;
							argptr = NULL;
						}
public:
	std::string			to;				// if this is "mbox", arg1 is mbox number, submsg is operation("set", "add", "mul") and arg2 is value
    void				*target;		// if not null, only applies to this alien
	std::string			msg;
	std::string			submsg;
	std::string			arg0;
	float				arg1;
	float				arg2;
	void				*argptr;
public:
	static std::vector<TMessage>	pending_messages;
	static void			queue_message(const char *_to, const char *_msg, const char *_submsg="", const char *_arg0="", float _arg1=0, float _arg2=0, void *_target=NULL) {
#ifdef SHOW_MESSAGE_DETAIL
printf("TMessage::queue_message(%s) %s: %s %s %g,%g 0x%016llx\n", _to, _msg, _submsg, _arg0, _arg1, _arg2, (long)(void *)_target);
#endif // SHOW_MESSAGE_DETAIL
        					int i = (int)pending_messages.size();
        					pending_messages.resize(i+1);
        					TMessage *m = &pending_messages[i];
        					m->to = _to;
        					m->msg = _msg;
        					if (strlen(_submsg) > 0) m->submsg = _submsg;
        					if (strlen(_arg0) > 0) m->arg0 = _arg0;
        					m->arg1 = _arg1;
        					m->arg2 = _arg2;
                            m->target = _target;
                        }
    static void			queue_message(TMessage &msg) {
#ifdef SHOW_MESSAGE_DETAIL
printf("TMessage::queue_message(%s) %s: %s %s %g,%g 0x%016llx\n", msg.to.c_str(), msg.msg.c_str(), msg.submsg.c_str(), msg.arg0.c_str(), msg.arg1, msg.arg2, (long)(void *)msg.target);
#endif // SHOW_MESSAGE_DETAIL
        					pending_messages.push_back(msg);
                        }
public:                 // select based on msg.to; "alien"/"mbox" means a first-class object with msg==entityname; "level" means the current level; "game" means game_state_dispatcher
	static void			send_message(TMessage &msg);		// intended to be called at update time (and maybe just before saving the state so no messages need be stored)
public:
	static void			update_send_messages() {
#ifdef SHOW_MESSAGE_DETAIL
if (pending_messages.size() > 0) printf("TMessage::update_send_messages() -----------------------------\n");
#endif // SHOW_MESSAGE_DETAIL
                            int i=0;
                            while (true) {
                                if (i >= pending_messages.size()) break;
                                send_message(pending_messages[i]);
                                i++;
                            }
                            pending_messages.resize(0);
                        }
};


#endif // !_TMESSAGE_H


