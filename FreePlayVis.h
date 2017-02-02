//
//  FreePlayVis.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _FREEPLAYVIS_H_
#define _FREEPLAYVIS_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>

#include "TAtlas.h"
#include "LElement.h"



#define VISUALIZER_NOTE			0
#define VISUALIZER_CENTER		1
#define VISUALIZER_OUTSIDE		2
#define VISUALIZER_IMMEDIATE_TOGGLE		3
#define VISUALIZER_DEFERRED_TOGGLE		4
#define VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE		5
#define VISUALIZER_ROTATE		6

// used in TRecordedTrack.cpp to determine default value of muteViz:
#define GAME_SELECT_PREVIEW			1
#define GAME_SELECT_BLASTER			2
#define GAME_SELECT_CENTERFIRE		3
#define GAME_SELECT_PREVIEW_3D		4
#define GAME_SELECT_FREE_PLAY		5


class TPartimitterInstance;	// fwd

class TActiveTouch_Visualizer : public TActiveTouch {			// this is how to append new fields to TActiveTouch (created only when "claiming" the touch, so no other elements will ever see it)
public:
						TActiveTouch_Visualizer() { disable_toggle_note_off = false; firstNote = -1; firstFlags = 0; previousNote = -1; previousFlags = 0; }
public:
	int					flags;									// item number in the touchesBegan content[] item, or -4..-1 for specials
//	int					flags_cur;								// item number in the touchesMoved/touchesEnded content[] item, or -4..-1 for specials
	int					note;									// index into note[] of the touchesBegan point
	int					octave;									// 
//	int					note_cur;								// index into note[] of the touchesMoved/touchesEnded point
public:
/*
	bool	drag_disable;
	bool	dont_set_last_used;
	int		touch_item;			// -1 for nothing, 0-3 for the 4 corners, 4 for center 
	int		cur_touch_item;		// same as above, changes while dragging

	CGPoint point;
	CGPoint actualpoint;		// the real point for tracking
//	int		note;
//	int		octave;
	int		flags;
	
	CGPoint	previousPoint;	
*/
	bool	dont_set_last_used;

	int		firstNote;
	int		firstFlags;

	int		previousNote;
	int		previousFlags;
	int		previousOctave;

	float	first_position_x;
	float	first_position_y;
	bool	disable_toggle_note_off;

	double	time_stamp;
	float	lastx, lasty;
public:			// 3d
};


class FreePlayVis : public LElement {		// LElement that manages a stack of menus/lists
public:
								FreePlayVis() { init(); }
	virtual						~FreePlayVis() { }
protected:
	virtual void				init();
	bool						is_paused;
public:
	TAtlas						*mdna_atlas;
public:		// interface to LElement:
	virtual void				update(double now, float frame_time);
	virtual TActiveTouch		*touch(int phase, double now, TActiveTouch *t, bool is_inside);	// return a new TActiveTouch_Choice to "claim" the touch (and block further traversal)
	virtual void				render(double now);
	virtual bool				message(const std::string &msg, float arg1, float arg2);
	virtual void				cleanup_on_exit();
public:
	virtual bool				getPaused() { return is_paused; }
	virtual void				rotateShape(int delta);
	virtual void				setNoteOn(int note, int volume, int nOctave, int art_selector, float art_alpha);
	virtual void				setNoteOff(int note, int nOctave);
};


void init_freeplay();



#endif // !_FREEPLAYVIS_H_
