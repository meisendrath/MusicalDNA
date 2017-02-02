//
//  FP_SYNTH.h
//

#ifndef _FP_SYNTH_H_
#define _FP_SYNTH_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>


typedef struct VoiceRequest {
    char program;				// 0..127: program index; 128 is visualizer, 129 is percussion (if enabled)
	char sf_patch;				// the specific voice to use from the soundfont (except percussion mode, s/b 0) (should match program index btw 0..127)
	char channel;				// 255 is visualizer
	char spare;
} VoiceRequest;

void FP_SYNTH_bootup_init();				// hardware boot
void FP_SYNTH_setup_with_voices(std::vector<VoiceRequest> &voicereqs);
void FP_SYNTH_all_notes_off();
void FP_SYNTH_set_patch(int ch, int inst, bool is_from_visualizer=false);
int FP_SYNTH_get_patch(int ch, bool is_from_visualizer=false);
void FP_SYNTH_rawplay(int ch, int notenum, int vol, bool is_from_visualizer=false);
void FP_SYNTH_rawplay_drums(int ch, int notenum, int vol);
void FP_SYNTH_control(int ch, int ctrl, int data, bool is_from_visualizer=false);
void FP_SYNTH_pitchbend(int ch, int data, bool is_from_visualizer=false);
void FP_SYNTH_set_global_volume(float vol);
float FP_SYNTH_get_global_volume();

#endif // !_FP_SYNTH_H_
