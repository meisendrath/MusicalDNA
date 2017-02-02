#include "FP_ROUTING.h"

#include "FP_SYNTH.h"
#include "FP_VISUALIZER.h"

#include "ios/MIDI.h"

#include "FPGameStatus.h"
extern FPGameStatus gGS;



static const char *src(int from) {
	if (from==ROUTING_FROM_NOTE_PLAYER) return "PLAYER";
	if (from==ROUTING_FROM_TOUCH_INPUT) return "TOUCH ";
	if (from==ROUTING_FROM_MIDI_INPUT)  return "MIDI  ";
	if (from==ROUTING_FROM_APPLICATION) return "APP   ";
	if (from==ROUTING_FROM_CF_SEARCH)   return "CF    ";
	if (from==ROUTING_FROM_CF2_SEARCH)  return "CF2   ";
	if (from==ROUTING_FROM_BL_SEARCH)   return "BL    ";
	if (from==ROUTING_FROM_BL2_SEARCH)  return "BL2   ";
	return "???";
}

#define RT { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }
#define o { -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1 }
typedef int ROUTING_BLOCK_TABLE_TYPE[ROUTING_FROM_MAX][ROUTING_TO_MAX][16];		// 16 channels
ROUTING_BLOCK_TABLE_TYPE ROUTING_BLOCK_TABLE = {
//     S   D   S   R   V   M   C   C   B   B   P  <-- TO
//     Y   R   Y   E   I   I   F   F   L   L   A
//     N   U   V   C   Z   D       2       2   R
//     T   M   I           I                   T
//     H   S   Z                               C
	{ RT,  o,  o, RT, RT, RT,  o,  o,  o,  o,  o },		// ROUTING_FROM_APPLICATION
	{ RT,  o,  o,  o, RT, RT,  o,  o,  o,  o,  o },		// ROUTING_FROM_NOTE_PLAYER
	{  o,  o, RT, RT, RT, RT,  o,  o,  o,  o,  o },		// ROUTING_FROM_TOUCH_INPUT		?? what channel/pgm/vol to synth, viz and MIDI
	{ RT,  o,  o, RT, RT,  o,  o,  o,  o,  o,  o },		// ROUTING_FROM_MIDI_INPUT
	{  o,  o, RT, RT, RT, RT,  o,  o,  o,  o,  o },		// ROUTING_FROM_CF_SEARCH		// pick up channel/pgm/vol from gRecordedSong->tracks match
	{ RT,  o,  o, RT, RT,  o,  o,  o,  o,  o,  o },		// ROUTING_FROM_CF2_SEARCH		// pick up channel/pgm/vol from gRecordedSong->tracks match
	{  o,  o, RT, RT, RT, RT,  o,  o,  o,  o,  o },		// ROUTING_FROM_BL_SEARCH
	{ RT,  o,  o, RT, RT,  o,  o,  o,  o,  o,  o },		// ROUTING_FROM_BL2_SEARCH
};
// centerfire playing: (route TOUCH_INPUT and MIDI_INPUT through CF_SEARCH and CF2_SEARCH)
//	{ RT,  o,  o, RT, RT, RT,  o,  o },		// ROUTING_FROM_APPLICATION
//	{ RT,  o,  o,  o, RT, RT,  o,  o },		// ROUTING_FROM_NOTE_PLAYER
//	{  o,  o,  o,  o,  o,  o, RT,  o },		// ROUTING_FROM_TOUCH_INPUT
//	{  o,  o,  o,  o,  o,  o,  o, RT },		// ROUTING_FROM_MIDI_INPUT
//	{  o,  o, RT, RT, RT, RT,  o,  o },		// ROUTING_FROM_CF_SEARCH		// pick up channel/pgm/vol from gRecordedSong->tracks match
//	{ RT,  o,  o, RT, RT,  o,  o,  o },		// ROUTING_FROM_CF2_SEARCH		// pick up channel/pgm/vol from gRecordedSong->tracks match

ROUTING_BLOCK_TABLE_TYPE *ROUTING_CUR_TABLE = &ROUTING_BLOCK_TABLE;

std::string ROUTING_serialize() {
	std::string s;
	for (int from=0; from<ROUTING_FROM_MAX; from++) {
		for (int to=0; to<ROUTING_TO_MAX; to++) {
			for (int ch=0; ch<16; ch++) {
				int c = (*ROUTING_CUR_TABLE)[from][to][ch];
				switch (c) {
					case  0:	s += "+";				break;
					case -1:	s += "o";				break;
					default:	s += (char)('0'+c);		break;
				}
			}
			s += " ";
		}
		s += "\n";
	}
	return s;
}

void ROUTING_deserialize(std::string s) {
	const char *ss = s.c_str();
	for (int from=0; from<ROUTING_FROM_MAX; from++) {
		for (int to=0; to<ROUTING_TO_MAX; to++) {
			for (int ch=0; ch<16; ch++) {
				switch (*ss) {
					case '+':	(*ROUTING_CUR_TABLE)[from][to][ch] = 0;				break;
					case 'o':	(*ROUTING_CUR_TABLE)[from][to][ch] = -1;			break;
					default:	(*ROUTING_CUR_TABLE)[from][to][ch] = *ss - '0';		break;
				}
				ss++;
			}
			if (*ss == ' ') ss++;
		}
		if (*ss == '\n') ss++;
	}
}

// ROUTING table: given a "from", determine which routines to call (maybe based on channel/track filtering)
//  returns true if the routes connect
inline bool route_test(int from, int to, int ch, bool track_muted=false, bool track_vizmuted=false) {
	int mode = (*ROUTING_CUR_TABLE)[from][to][ch];
	if (mode == RB_ALL) return true;
	if (mode == RB_NONE) return false;
	if (mode == 2) return !track_muted;			// honor track_muted, otherwise true
	if (mode == 3) return !track_vizmuted;		// honor track_vizmuted, otherwise true
	if (ch >= 16) {
printf("route_test() called for channel >= 16!!!\n");
		return false;
	}
	return 0 == (mode & (1 << ch));
}

void ROUTING_dump() {
	printf("                   SYN               DRUM              SVIZ              REC               VIZ               MIDI              CF                CF2               BL                BL2           PARTICLES \n");
	for (int from=0; from<ROUTING_FROM_MAX; from++) {
		printf("    %s ", src(from));
		for (int to=0; to<ROUTING_TO_MAX; to++) {
			printf("  ");
			for (int ch=16; --ch>=0;) switch ((*ROUTING_CUR_TABLE)[from][to][ch]) {
				case RB_NONE:			printf("o");			break;
				case RB_ALL:			printf("+");			break;
				case 2:					printf("2");			break;
				case 3:					printf("3");			break;
				default:				printf("%d ", (*ROUTING_CUR_TABLE)[from][to][ch]);			break;
			}
		}
		printf("\n");
	}
}


void ROUTING_set_route(int from, int to, int channelmask) {
#ifdef ROUTING_VERY_VERBOSE
printf(" *** ROUTING_set_route FROM %s: TO %s: new_value=%d was %d [0]\n", src(from), dst(to), channelmask, (*ROUTING_CUR_TABLE)[from][to][0]);
#endif // ROUTING_VERY_VERBOSE
	for (int ch=0; ch<16; ch++)
		(*ROUTING_CUR_TABLE)[from][to][ch] = channelmask;
}

void ROUTING_set_active(int from, int to, int ch, int is_active) {
#ifdef ROUTING_VERY_VERBOSE
printf(" *** ROUTING_set_active FROM %s: TO %s: new_value=%d ch=%d was=%d\n", src(from), dst(to), is_active, ch, ((*ROUTING_CUR_TABLE)[from][to][ch]));
#endif // ROUTING_VERY_VERBOSE
	(*ROUTING_CUR_TABLE)[from][to][ch] = is_active;
}


void ROUTING_note(int from, bool noteOn, int ch, int note, int vol, int arg1, float arg2, bool track_sez_MUTE_AUDIO, bool track_sez_MUTE_VIZ) {
#ifdef ROUTING_VERBOSE
printf(" *** NOTE ROUTING FROM %s: ch%01x %02x %02x --> ", src(from), ch, note, vol);
for (int ito=0; ito<ROUTING_TO_MAX; ito++) if (route_test(from, ito, ch)) printf("%s ", dst(ito));
printf("\n");
#endif // ROUTING_VERBOSE
	if (!track_sez_MUTE_AUDIO) {
		if (route_test(from, ROUTING_TO_SYNTH, ch)) FP_SYNTH_rawplay(ch, note, vol);
		if (route_test(from, ROUTING_TO_SYNTH_DRUMS, ch)) FP_SYNTH_rawplay_drums(ch, note, vol);
		if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, ch)) FP_SYNTH_rawplay(ch, note, vol, true);
	}
	if (!track_sez_MUTE_VIZ)
		if (route_test(from, ROUTING_TO_VIZUALIZER, ch)) FP_VISUALIZER_note(ch, note, vol, ch, arg1, arg2, noteOn);
	if (route_test(from, ROUTING_TO_MIDI_OUT, ch)) {
		char msg[4];
		msg[0] = (char)(0x80+ch+((vol==0)?0:0x10));
		msg[1] = (char)note;
		msg[2] = (char)vol;
		MIDI_send(msg, 3);
	}
}



void ROUTING_all_notes_off(int from) {
#ifdef ROUTING_VERBOSE
printf(" *** ALL NOTES OFF FROM %s:", src(from));
printf("    --> ");
for (int ito=0; ito<ROUTING_TO_MAX; ito++) if (route_test(from, ito, 0)) printf("%s ", dst(ito));
printf("\n");
#endif // ROUTING_VERBOSE
	if (route_test(from, ROUTING_TO_SYNTH, 0)) FP_SYNTH_all_notes_off();
	if (route_test(from, ROUTING_TO_SYNTH_DRUMS, 0)) FP_SYNTH_all_notes_off();
	if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, 0)) FP_SYNTH_all_notes_off();
	if (route_test(from, ROUTING_TO_VIZUALIZER, 0)) FP_VISUALIZER_allnotesoff();
	if (route_test(from, ROUTING_TO_MIDI_OUT, 0)) {
		char msg[4];
		msg[0] = (char)0xb0;
		msg[1] = (char)0x7b;
		msg[2] = (char)0x00;
		MIDI_send(msg, 3);
	}
}

void ROUTING_set_patch(int from, int ch, int inst) {
#ifdef ROUTING_VERBOSE
printf(" *** PATCH ROUTING FROM %s: ch%01x %02x", src(from), ch, inst);
printf("    --> ");
for (int ito=0; ito<ROUTING_TO_MAX; ito++) if (route_test(from, ito, ch)) printf("%s ", dst(ito));
printf("\n");
#endif // ROUTING_VERBOSE
	if (route_test(from, ROUTING_TO_SYNTH, 0)) FP_SYNTH_set_patch(ch, inst, false);
	if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, 0)) FP_SYNTH_set_patch(ch, inst, true);
	if (route_test(from, ROUTING_TO_MIDI_OUT, 0)) {
		char msg[4];
		msg[0] = (char)(0xc0 + ch);
		msg[1] = (char)inst;
		MIDI_send(msg, 2);
	}
}

int ROUTING_get_patch(int from, int ch) {
	if (route_test(from, ROUTING_TO_SYNTH, 0)) return FP_SYNTH_get_patch(ch, false);
	if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, 0)) return FP_SYNTH_get_patch(ch, true);
	return 0;
}

void ROUTING_pitch_bend(int from, int ch, int data, bool force_viz) {
#ifdef ROUTING_VERBOSE
printf(" *** PITCH BEND ROUTING FROM %s: ch%01x %d%s", src(from), ch, data, force_viz?" VIZ":"");
printf("    --> ");
for (int ito=0; ito<ROUTING_TO_MAX; ito++) if (route_test(from, ito, ch)) printf("%s ", dst(ito));
printf("\n");
#endif // ROUTING_VERBOSE
	if (force_viz) {
		FP_SYNTH_pitchbend(ch, data, true);
	} else {
		if (route_test(from, ROUTING_TO_SYNTH, 0)) FP_SYNTH_pitchbend(ch, data, false);
		if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, 0)) FP_SYNTH_pitchbend(ch, data, true);
	}
}

void ROUTING_control(int from, int ch, int ctrl, int data) {
#ifdef ROUTING_VERBOSE
printf(" *** CONTROL ROUTING FROM %s: ch%01x %02x %02x", src(from), ch, ctrl, data);
printf("    --> ");
for (int ito=0; ito<ROUTING_TO_MAX; ito++) if (route_test(from, ito, ch)) printf("%s ", dst(ito));
printf("\n");
#endif // ROUTING_VERBOSE
	if (route_test(from, ROUTING_TO_SYNTH, 0)) FP_SYNTH_control(ch, ctrl, data, false);
	if (route_test(from, ROUTING_TO_SYNTH_VIZ_VOICE, 0)) FP_SYNTH_control(ch, ctrl, data, true);
	if (route_test(from, ROUTING_TO_MIDI_OUT, 0)) {
		char msg[4];
		msg[0] = (char)(0xb0 + ch);
		msg[1] = (char)ctrl;
		msg[2] = (char)data;
		MIDI_send(msg, 3);
	}
}

