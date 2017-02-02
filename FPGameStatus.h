
#ifndef _FPGAMESTATUS_H
#define _FPGAMESTATUS_H

#include "config.h"
#include "common.h"

class FPGameStatus {
public:
    FPGameStatus() {
                           reset_all();
    }
    void                   reset_all() {
								reset();
								user_screen_flip = false;
                                user_tablet_mode = true;
                                user_sound_effects = true;
								user_local_synth_off = false;
								user_star_field = true;
    }
    void                   reset() {
								visualizer_instrument_index = 0;
								visualizer_volume = 1.0f;
								visualizer_art_index = 0;
								visualizer_art_alpha = 1.0f;
								visualizer_channel = 0;
    }
public:	// general
    bool					user_screen_flip, user_tablet_mode, user_sound_effects, user_star_field, user_local_synth_off;
public:		// settings for the visualizer
    int						visualizer_instrument_index;
    float					visualizer_volume;
    int						visualizer_art_index;
    float					visualizer_art_alpha;
    int						visualizer_channel;
    
    double					gGameState_start_time;				//
    
};

#endif
