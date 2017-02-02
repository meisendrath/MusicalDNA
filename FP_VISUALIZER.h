//
//  FP_VISUALIZER.h
//

#ifndef _FP_VISUALIZER_H_
#define _FP_VISUALIZER_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>




void FP_VISUALIZER_init();
void FP_VISUALIZER_note(int ch, int note, int vel, int track, int art_index, float art_alpha, bool noteOn);
void FP_VISUALIZER_allnotesoff();			// responds to ROUTING by resetting internal gViz[] structure
void FP_VISUALIZER_allgviznotesoff();		// calls ROUTING to turn off active notes
int FP_VISUALIZER_isNoteActiveOnAnyChannel(int note);
void FP_VISUALIZER_recent_notes_off();
void FP_VISUALIZER_update(float frame_time);
class TAtlas;
void FP_VISUALIZER_render_sprite_or_seq(const char *name, int curframe, float x, float y, TAtlas *atlas, float scale=1.0f, float rotation=0, float r=1, float g=1, float b=1, float a=1, int rendermode=0);
void FP_VISUALIZER_render(TAtlas *mdna_atlas, float cx, float cy, float cr, float r, bool show_centerfire, float scale=1.0f);

void FP_VISUALIZER_guarantee_this_many_tracks(int n);



#define MAX_INSTRUMENTS 130
TAtlas* FP_VISUALIZER_get_line_atlas(int art_index);
TAtlas* FP_VISUALIZER_get_note_atlas(int art_index);
TAtlas* FP_VISUALIZER_get_anim_atlas(int art_index);
bool FP_VISUALIZER_exist_atlas(int cno);
float FP_VISUALIZER_get_art_alpha(int cno);


#endif // !_FP_VISUALIZER_H_
