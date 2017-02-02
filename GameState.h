//
//  SState.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>

#include "TAtlas.h"
#include "TMessage.h"
#include "LMList.h"


//SState: game state base class showing message handler, etc
//  - has a ref to another state when saving for later resume
class GameState {
public:
    GameState() { psd_file = NULL; /*gStates.add(this);*/ }
	virtual						~GameState() { gStates.remove(this); }
public:
	std::string					name;							// state name
    int							ident;							// old-school state id
    //protected:		// overlay state might need to access parent's psd_file
	TAtlas						*psd_file;
    std::string					psd_file_name;
public:
	virtual void				get_name(char *out_ptr) { sprintf(out_ptr, "%s", name.c_str()); }
	virtual void				update(double now, float frame_time) { }
	virtual void				render_underlay(double now) { }
	virtual void				render(double now) { }
	virtual void				render_overlay(double now) { }
	virtual void				render_callback(int ref);
	virtual void				enter(double now, float arg1) { }
	virtual void				resume_enter(double now, float arg1) { }
	virtual void				exit(double now) { }
	virtual void				transition_done(double now) { }			// already appended all elements from LElement::gSliding_on onto [END OF] LElement::gScene
public:
	virtual bool				message(const std::string &msg, float arg1, float arg2) { return false; }
	virtual bool				message(const TMessage &msg) { return message(msg.msg, msg.arg1, msg.arg2); }
    static void					send(const char *msg, float arg=0);
    static void					send2(const char *msg, float arg1, float arg2);
    static void					sendstr(const char *msg, const std::string &arg, float arg1=0);
    void						deliver_message(const TMessage &msg);
	
    static void					change_state(const char *new_state_name, float arg=0);
    virtual void				change_state_back(float arg=0);
	static void					imm_message_to_state(const char *state_name, const char *msg, float arg);
	static GameState				*get_state_by_ident(int ident);
	static GameState				*get_state_by_name(const std::string &nm);
public:
	virtual void				read_psd(bool ignore_aspect_ratio=false, const char *psdname=NULL);
public:
	static LMList<GameState>		gStates;
    static void                 free_play_dispatcher_create_states();
};


#endif // !_SSTATE_H_
