#include "FP_VISUALIZER.h"

#include "FP_SYNTH.h"
#include "FP_ROUTING.h"

#include "HAL.h"
#include "TAtlas.h"
#include "LElement.h"		// for send()
#include "math.h"




extern HAL *gHAL;
extern double getcurrenttime();
extern double start_time;


extern int gScreenWidth, gScreenHeight;
extern double curtimeval;
extern double curtimeval_p;
extern float gScreenAspectRatio;



double curtime_pause = 0;
double start_time;


#define M_2PI			(3.1415926535897932384626433 * 2.0)
#define LINE_FADE_TIME 0.5f
#define NOTE_FADE_TIME 0.5f				
#define NOTE_FADE_IN_TIME 0.05f				


struct ActiveLine {	
	int track;
	int instrument_index;
	int art_index;
	int note1;						// 0..11
	int note2;
	int notenum1;					// note number
	int notenum2;
	int line_length;
	int is_melody;
	int done_melody;
	int orphan;						// 1 if both notes were found to be off
	float art_alpha;
	double start_time;
	double last_time;
};
static std::vector<ActiveLine>			m_ActiveLines;



class TVizTrackData {
public:
					TVizTrackData() { instrument_index = 0; art_index = 0; art_alpha = 1.0f; reset(0); }
	int				note_active[12];
	int				note_was_active[12];	// same as note_active, except not cleared by note off
	double			note_timestamp[12];
	double			note_end_timestamp[12];
	int				instrument_index;		// copy of the program assigned to a particular channel when played
	int				art_index;
	float			art_alpha;
public:
	void			reset(double timeStamp) {
		for (int i=0; i<12; i++) {
			note_active[i] = 0;
			note_was_active[i] = 0;
			note_end_timestamp[i] = note_timestamp[i] = timeStamp;
		}
	}
};
static std::vector<TVizTrackData>		gViz;

int note_number_recent[12];		// note numbers of "recent" notes

static double time_passed[12];
double chord_id_time;

static bool			show_recent = false;

static bool			viz_initted = false;


static TAtlas* visualizer_line_atlas[MAX_INSTRUMENTS];
static TAtlas* visualizer_anim_atlas[MAX_INSTRUMENTS];
static TAtlas* visualizer_note_atlas[MAX_INSTRUMENTS];




TAtlas* FP_VISUALIZER_get_line_atlas(int cno) {
	TAtlas *a = visualizer_line_atlas[gViz[cno].art_index];
	if (a == NULL) a = visualizer_line_atlas[0];
	return a;
}
TAtlas* FP_VISUALIZER_get_note_atlas(int cno) {
	TAtlas *a = visualizer_note_atlas[gViz[cno].art_index];
	if (a == NULL) a = visualizer_note_atlas[0];
	return a;
}
TAtlas* FP_VISUALIZER_get_anim_atlas(int cno) {
	TAtlas *a = visualizer_anim_atlas[gViz[cno].art_index];
	if (a == NULL) a = visualizer_anim_atlas[0];
	return a;
}
bool FP_VISUALIZER_exist_atlas(int cno) {
	if ((visualizer_line_atlas[cno] == NULL) && (visualizer_anim_atlas[cno] == NULL) && (visualizer_note_atlas[cno] == NULL)) return false;
	return true;
}
float FP_VISUALIZER_get_art_alpha(int cno) {
	return gViz[cno].art_alpha;
}


void FP_VISUALIZER_guarantee_this_many_tracks(int n) {
	if (gViz.size() < n) gViz.resize(n);
}

void FP_VISUALIZER_recent_notes_off() {
	memset( note_number_recent, 0, sizeof(note_number_recent) );	
}

void FP_VISUALIZER_init() {
	if (viz_initted) return;
	viz_initted = true;
	start_time = getcurrenttime();
	gViz.resize(16);
	for (int t=0; t<gViz.size(); t++) gViz[t].reset(start_time);
	for (int t=0; t<MAX_INSTRUMENTS; t++) visualizer_line_atlas[t] = visualizer_anim_atlas[t] = visualizer_note_atlas[t] = NULL;

	if (visualizer_line_atlas[0] == NULL) visualizer_line_atlas[0] = TAtlas::get_cached_or_load("instrument0_line");
	if (visualizer_anim_atlas[0] == NULL) visualizer_anim_atlas[0] = TAtlas::get_cached_or_load("instrument0_anim");
	if (visualizer_note_atlas[0] == NULL) visualizer_note_atlas[0] = TAtlas::get_cached_or_load("instrument0_note");

}



int FP_VISUALIZER_isNoteActiveOnAnyChannel(int note) {
    for (int t=0; t<gViz.size(); t++) 
        if (gViz[t].note_active[note % 12] != 0)
            return t;
    return -1;
}


void FP_VISUALIZER_allnotesoff() {
	double then = getcurrenttime() - 1000;
	for (int t=0; t<gViz.size(); t++) gViz[t].reset(then);
}

void FP_VISUALIZER_allgviznotesoff() {
	for (int tr=0; tr<gViz.size(); tr++) for (int i=0; i<12; i++) {
		int n = gViz[tr].note_active[i];
		if (n != 0) ROUTING_note(ROUTING_FROM_TOUCH_INPUT, false, tr, n, 0);
	}
}

// called from ROUTING_note() and therefore FP_VISUALIZER_setNoteOn()/FP_VISUALIZER_setNoteOff(), NEW_CONTROLLER_execute_event(), MDNAMIDIReadProc(), FP_VISUALIZER_allgviznotesoff()
void FP_VISUALIZER_note(int ch, int note, int vel, int track, int art_index, float art_alpha, bool noteOn) {
	if (gViz.size() < track+1) gViz.resize(track+1);
	gViz[track].note_active[note % 12] = noteOn?note:0;
	gViz[track].note_was_active[note % 12] = note;
	gViz[track].instrument_index = FP_SYNTH_get_patch(ch);
	gViz[track].art_index = art_index;
	gViz[track].art_alpha = art_alpha;
	if (noteOn) gViz[track].note_timestamp[note % 12] = getcurrenttime();
		else gViz[track].note_end_timestamp[note % 12] = getcurrenttime();

}

static void FP_VISUALIZER_add_line(ActiveLine line) {
//printf("FP_VISUALIZER_add_line %d to %d, ch:%d\n", line.note1, line.note2, line.track);
	// sort for 2..6, 1
	if (line.line_length == 1) m_ActiveLines.push_back( line );
	else {
		ActiveLine templine = line;
		for (int j=0; j<m_ActiveLines.size(); j++) {
			if (m_ActiveLines[j].line_length > line.line_length) {
				ActiveLine templine2;
				for (int k=j; k<m_ActiveLines.size(); k++) {
					templine2 = m_ActiveLines[k];
					m_ActiveLines[k] = templine;
					templine = templine2;
				}
				break;
			}
		}
		m_ActiveLines.push_back(templine);
	}
}

static void FP_VISUALIZER_gather_lines(int linelen, int track, float scale) {
	double now = getcurrenttime();
	for (int idx=0; idx<12; idx++) {
		int idx2a = idx+linelen;
		int idx2 = idx2a % 12;
		int idx1 = idx;
		if ((gViz[track].note_active[idx1] == 0) && (gViz[track].note_active[idx2] == 0)) continue;	// skip out if both of these notes are not active
		ActiveLine newline;
		newline.track = track;
		newline.instrument_index = gViz[track].instrument_index;
		newline.art_index = gViz[track].art_index;
		newline.art_alpha = gViz[track].art_alpha;
		newline.note1 = idx1;
		newline.note2 = idx2;
		newline.notenum1 = gViz[track].note_was_active[idx1];
		newline.notenum2 = gViz[track].note_was_active[idx2];
		newline.line_length = linelen;
		newline.start_time = newline.last_time = now;
		newline.is_melody = 0;
		newline.done_melody = 0;
		newline.orphan = 0;
		
		bool bFound = false;
		if (gViz[track].note_active[idx2] == 0) {					// second is inactive, swap
			int tmp = idx1;
			idx1 = idx2;
			idx2 = tmp;
			newline.note1 = idx2;
			newline.note2 = idx1;
			newline.notenum1 = gViz[track].note_was_active[idx2];
			newline.notenum2 = gViz[track].note_was_active[idx1];
		}
		//
		if (gViz[track].note_active[idx1] == 0) {					// melody or nothing

			int most_recent_release;
			// bail if this is not the most recent release
			double t = gViz[track].note_timestamp[idx1];
			most_recent_release = idx1;
			for (int j=0; j<12; j++) {
				if ((j != idx1) && (j != idx2)) {
					if (t < gViz[track].note_timestamp[j]) {
						most_recent_release = j;
					}
				}
			}
			if (idx1 != most_recent_release) continue;



// ONLY ADD THE LINE TO THE MOST RECENT NOTE BEFORE idx1
// if we can find a note thats between idx1 and idx2, bail
			most_recent_release = idx1;
			for (int j=0; j<12; j++) {
				if ((j != idx1) && (j != idx2)) {
					if ((gViz[track].note_timestamp[idx1] <= gViz[track].note_timestamp[j]) &&
						(gViz[track].note_timestamp[idx2] >= gViz[track].note_timestamp[j])) {
						most_recent_release = j;
					}
					if ((gViz[track].note_timestamp[idx1] >= gViz[track].note_timestamp[j]) &&
						(gViz[track].note_timestamp[idx2] <= gViz[track].note_timestamp[j])) {
						most_recent_release = j;
					}
				}
			}
			if (idx1 != most_recent_release) continue;


			if ((fabsf((float)(now - gViz[track].note_end_timestamp[idx1])) < 0.5f) && (fabsf((float)(now - gViz[track].note_timestamp[idx2])) < 0.5f)) {		// *** why 0.5f???
				// find and convert possible chord line mistakenly connecting these two notes
				for( int i = 0; i < m_ActiveLines.size(); i++ )
				{
					ActiveLine& line = m_ActiveLines[i];
					if( line.track == track && line.is_melody == 0 && line.orphan == 0 && ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
					{
						// found the line
						bFound = true;
						newline.is_melody = 1;
						m_ActiveLines[i] = newline;		// replace with melody line
						break;
					}
					if( line.track == track && line.is_melody == 1 && line.orphan == 0 && ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
					{
						// found the line
						bFound = true;
						break;
					}
					
				}
				
				if( !bFound )
				{
					newline.is_melody = 1;
					newline.start_time = gViz[track].note_timestamp[idx2];
					if (gViz[track].note_active[idx2] != 0) {
						if (gViz[track].note_timestamp[idx2] > gViz[track].note_timestamp[idx1]) {
							int tmp = idx1;
							idx1 = idx2;
							idx2 = tmp;
							newline.note1 = idx2;
							newline.note2 = idx1;
							newline.notenum1 = gViz[track].note_was_active[idx2];
							newline.notenum2 = gViz[track].note_was_active[idx1];
						}
					}
					FP_VISUALIZER_add_line(newline);
				}
			}
		} else
		{													// both notes still active, it's a chord line
			for( int i = 0; i < m_ActiveLines.size(); i++ )
			{
				ActiveLine& line = m_ActiveLines[i];
				if( line.track == track && line.is_melody == 0 && ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
				{
					// found the line - update the time stamp
					bFound = true;
					line.last_time = getcurrenttime();
					break;
				}
				
			}
			
			if( !bFound )
			{
				// line not found - add it to the list
				FP_VISUALIZER_add_line(newline);
			}
		}
	}
}

static void FP_VISUALIZER_render_active_line(ActiveLine &line, unsigned int curframe, float fade_value, float r, float scale) {

	float xx = 0;									// center of the circle
	float yy = 0;
	float ang = (float)M_2PI * ((line.note1%12)/12.0f);
	float x = xx + r * sinf(ang);
	float y = yy - r * cosf(ang);
	// lines
	float ang2 = (float)M_2PI * ((line.note2%12)/12.0f);
	float x2 = xx + r * sinf(ang2);
	float y2 = yy - r * cosf(ang2);
	float x3 = x2 + x;
	x3 *= 0.5f;
	float y3 = y2 + y;
	y3 *= 0.5f;
	
	fade_value *= line.art_alpha;

	gHAL->matrix_push();
	gHAL->matrix_translate(x3, y3, 0);
	float ang3 = atan2f(y2 - y, x2 - x);
	gHAL->matrix_rotate((ang3/(float)M_2PI) * 360, 0, 0, 1);
	int seq;
	float rlen = 240;
	switch (line.line_length) {
		case 1: case 11:		seq = 1;		rlen = 62;		break;
		case 2:	case 10:		seq = 2;		rlen = 120;		break;
		case 3: case  9:		seq = 3;		rlen = 170;		break;
		case 4: case  8:		seq = 4;		rlen = 208;		break;
		case 5: case  7:		seq = 5;		rlen = 232;		break;
		case 6:					seq = 6;		rlen = 240;		break;
	}
	float dist_x_x2 = sqrtf((x2-x)*(x2-x) + (y2-y)*(y2-y));
	float scale2 = scale * dist_x_x2 / rlen;
	{
		int w = visualizer_line_atlas[line.art_index]->get_atlas_sequence_frame_width(seq, 0);
		int h = visualizer_line_atlas[line.art_index]->get_atlas_sequence_frame_height(seq, 0);
		w -= h;		// 1/2 h from the left, to 1/2 h before the right edge
		scale2 *= rlen / w;
	}
	gHAL->matrix_scale(scale2, scale2, 1);		// y*4 to widen the line
if (line.track >= gViz.size()) printf("ERROR ERROR ERROR: referring to a track %d not present in gViz[%d]!!!\n", line.track, (int)gViz.size());
	if (line.track < gViz.size()) {
		if (line.is_melody == 0) {
			TAtlas* atlas = visualizer_line_atlas[line.art_index];
			if (atlas == NULL) atlas = visualizer_line_atlas[0];
		
			if (atlas != NULL) {
				atlas->draw_atlas_sequence_center(seq, curframe, 0, 0, 1, 1, 1, fade_value, 1);
			}

		} else {
			TAtlas* atlas = visualizer_anim_atlas[line.art_index];
			if (atlas == NULL) atlas = visualizer_anim_atlas[0];

			if (atlas != NULL) {
				int maxframe = atlas->get_atlas_num_frames_for_sequence(seq);
				if (curframe >= maxframe) {
					curframe = maxframe - 1;
					line.done_melody = 1;
				}

				atlas->draw_atlas_sequence_center(seq, curframe, 0, 0, 1, 1, 1, fade_value, 1);
			}
		}
	}

	gHAL->matrix_pop();
}

static void FP_VISUALIZER_render_active_line(int i, float r, float scale=1.0f) {
	double now = getcurrenttime();
	float fade_value = 1;
	unsigned int curframe;
	if (m_ActiveLines[i].is_melody == 0) {
		double time_last = now - m_ActiveLines[i].last_time;
		fade_value = (float)(time_last / LINE_FADE_TIME);
		fade_value = 1.0f - fade_value;
		if (fade_value < 0) fade_value = 0;
		curframe = (int)(fmin(time_passed[m_ActiveLines[i].note1%12], time_passed[m_ActiveLines[i].note2%12]) / (1 / 30.0f));		// convert to frames at 30 FPS
	} else {
		curframe = (int)((now - m_ActiveLines[i].start_time) / (1 / 30.0f));		// convert to frames at 30 FPS
	}
	FP_VISUALIZER_render_active_line(m_ActiveLines[i], curframe, fade_value, r, scale);
}

void FP_VISUALIZER_render_sprite_or_seq(const char *name, int curframe, float x, float y, TAtlas *atlas, float scale, float rotation, float r, float g, float b, float a, int rendermode) {
	gHAL->matrix_push();
	gHAL->matrix_translate(x, y, 0);
	gHAL->matrix_rotate(rotation, 0, 0, 1);
	gHAL->matrix_scale(scale, scale, 1);
	std::string fn;
	fn = name;
	fn += ".png";
	int f = atlas->get_atlas_frame_number_for_file_name(fn.c_str(), true);
	if (f < 0) {
		fn = name;
		fn += "00.png";
		f = atlas->get_atlas_frame_number_for_file_name(fn.c_str());
		if (f >= 0) {
			int seq = atlas->get_atlas_sequence_for_frame(f);
			atlas->draw_atlas_sequence_center(seq, curframe, 0, 0, r, g, b, a, rendermode);
		}
	} else {
		atlas->draw_atlas_texture_c(f, 0, 0, r, g, b, a, rendermode);
	}
	gHAL->matrix_pop();
}

static void FP_VISUALIZER_render_internal(double now, TAtlas *mdna_atlas, float r, float scale=1.0f) {
	// render the lines
	for (int i=0; i<m_ActiveLines.size(); i++) {
		FP_VISUALIZER_render_active_line(i, r);
	}
	
	// if no notes active, show 25% dimmed "recent" chord
	int nactive = 0;
	for (int cno=0; cno<gViz.size(); cno++)
		for (int lineno=0; lineno<12; lineno++)
			if (gViz[cno].note_active[lineno] != 0) nactive++;

	if (nactive == 0) {
		gHAL->matrix_push();
		for (int linelenidx=0; linelenidx<6; linelenidx++) {
			int linelen = 2 + linelenidx;
			if (linelen == 7) linelen = 1;
			int isactive[12];
			int isactiven1[12];
			int isactiven2[12];
			for (int j=0; j<12; j++) isactive[j] = 0;
			for (int idx=0; idx<12; idx++) {
				int idx2a = idx+linelen;
				int idx2 = idx2a % 12;
				int idx1 = idx;

				if ((note_number_recent[idx1] != 0) && (note_number_recent[idx2] != 0)) {
					if ((linelen != 6) || (note_number_recent[idx1] < note_number_recent[idx2])) {
						isactive[idx] = 1;
						isactiven1[idx] = note_number_recent[idx1];
						isactiven2[idx] = note_number_recent[idx2];
					}
				}
			}
			for (int j=0; j<12; j++) if (isactive[j] != 0) {
				ActiveLine line;
				line.is_melody = 0;
				line.orphan = 0;
				line.start_time = line.last_time = now;
				line.note1 = j;
				line.note2 = j + linelen;
				line.notenum1 = isactiven1[j];
				line.notenum2 = isactiven2[j];
				line.line_length = linelen;
// because this line is temporary, just for this render, we will not later refer to gViz[line.track], which would be incorrect...
				line.track = 0;
//				line.instrument_index = gGS.visualizer_instrument_index;
                line.instrument_index = 0;
//				line.art_index = gGS.visualizer_art_index;
                line.art_index = 0;
//				line.art_alpha = gGS.visualizer_art_alpha;
                line.art_alpha = 1.0f;
				unsigned int curframe = (int)((now - start_time) / (1 / 30.0f));		// convert to frames at 30 FPS
curframe = 0;		// *** why no anim???
				FP_VISUALIZER_render_active_line(line, curframe, 0.25f, r, 1.0f);		// *** why 0.25f???
			}
		}
		gHAL->matrix_pop();
	}


	// dots
	for (int idx=0; idx<12; idx++) {


		float xx = 0;									// center of the circle
		float yy = 0;
		float ang = (float)(M_2PI * (idx/12.0f));
		float x = xx + r * sinf(ang);
		float y = yy - r * cosf(ang);
		// min time passed since a state transition for a note
		unsigned int curframe = (int)((time_passed[idx] + 0.0023*idx) / (1 / (30.0f)));		// convert to frames at 30 FPS

		// check if this is a root
		bool bRoot = false;

//#warning root animation is not implemented
/*
		if( show_chord_id )
		{
			int roots[12];
			if( current_roots.size() ) {	
				for( int i = 0; i < current_roots.size(); i++ )	{
					if( current_roots[i] == idx ) {
						bRoot = true;
						break;
					}
				}
			}
		}
*/
		
		// dots 
		bool done = false;

		for (int t=0; t<gViz.size(); t++) if (gViz[t].note_active[idx] != 0 || ((now - gViz[t].note_end_timestamp[idx]) < NOTE_FADE_TIME) ) {
			float fade_value = 1;
			TAtlas* atlas = visualizer_note_atlas[gViz[t].art_index];
			if (atlas == NULL) atlas = visualizer_note_atlas[0];

			
			// calculate note fade
			if (gViz[t].note_active[idx] == 0) {
				fade_value = (float)((now - gViz[t].note_end_timestamp[idx]) / NOTE_FADE_TIME);
				fade_value = 1 - fade_value;
				if (fade_value < 0) fade_value = 0;
				if (fade_value > 1) fade_value = 1;
			}
            
            fade_value *= gViz[t].art_alpha;

			FP_VISUALIZER_render_sprite_or_seq(bRoot ? "dotroot" : "dothighlight", curframe, x, y, atlas, scale/4, idx*107, fade_value, fade_value, fade_value, fade_value, 1);
				
			done = true;

//			break;	// only draw the lowest-numbered active note found in gViz[] (comment this out to draw all track layers)
		}
        
		// recent notes
		if (!done) {
			if ((note_number_recent[idx] != 0) && show_recent) {
// use a global value for visualizer, since recent only goes with touches
//				TAtlas* atlas = visualizer_note_atlas[gGS.visualizer_art_index];
                TAtlas* atlas = visualizer_note_atlas[0];
				if (atlas == NULL) atlas = visualizer_note_atlas[0];

				FP_VISUALIZER_render_sprite_or_seq(bRoot ? "dotrootrecent" : "dotrecent", curframe, x, y, atlas, scale, idx*107);
				done = true;
			}
		}
	}
	
	// draw the particles

//#warning not drawing particles
//	if( enable_note_particles )	
//	{
//		for( int i = 0; i < 12; i++ )
//		{
//			if( m_pNoteParticles[i] ) m_pNoteParticles[i]->Draw();
////			if( m_pRootParticles[i] ) m_pRootParticles[i]->Draw();
//		}
//		glEnable(GL_TEXTURE_2D);
//	}
	
	


}


void FP_VISUALIZER_update(float frame_time) {
}


void FP_VISUALIZER_render(TAtlas *mdna_atlas, float cx, float cy, float cr, float r, bool show_centerfire, float scale) {
	double now = getcurrenttime();


// basic dots
	for (int idx=0; idx<12; idx++) {
		float xx = 0;//screen_width/2+kVisCenterX;									// center of the circle
		float yy = 0;//screen_height/2+kVisCenterY;
		float ang = (float)(M_2PI * (idx/12.0f));
		float x = xx + r * sinf(ang);
		float y = yy - r * cosf(ang);
		unsigned int curframe = (int)((time_passed[idx] + 0.0023*idx) / (1 / (30.0f)));		// convert to frames at 30 FPS
//#warning dot image from mdna_atlas
//		TAtlas* atlas = visualizer_note_atlas[gGS.visualizer_art_index];
        TAtlas* atlas = visualizer_note_atlas[0];
		if (atlas == NULL) atlas = visualizer_note_atlas[0];

		FP_VISUALIZER_render_sprite_or_seq("dot", curframe, x, y, atlas, scale/4);
	}
	

	// global checks: have the two most recent notes been turned on or off (same for each) within ~25ms? if so, don't draw ANY moving note animations
	// MOVING NOTE ANIMATIONS: 
	for (int idx=0; idx<12; idx++) {
		// min time passed since a state transition for a note
		time_passed[idx] = 99999999;
		for (int t=0; t<gViz.size(); t++) {
			double tp = now - gViz[t].note_timestamp[idx];
			if (time_passed[idx] > tp) time_passed[idx] = tp;
		}
	}

	// gather the lines
	for (int cno=0; cno<gViz.size(); cno++)
		for (int lineno=1; lineno<12; lineno++)
			FP_VISUALIZER_gather_lines(lineno, cno, scale);
	
	// time out the lines (melody lines can only be shown once through, chord lines fade after LINE_FADE_TIME)
	int jj=0;
	while (jj < m_ActiveLines.size()) {
		ActiveLine& line = m_ActiveLines[jj];
		bool killer = false;
		if (line.is_melody == 0) {
			if ((now - line.last_time) > LINE_FADE_TIME) killer = true;
		} else {
			if (line.done_melody != 0) killer = true;
		}
		if (killer) {
			m_ActiveLines.erase(m_ActiveLines.begin() + jj);
		} else {
			if ((gViz[line.track].note_active[line.note1] == 0) && (gViz[line.track].note_active[line.note2] == 0)) line.orphan = 1;
			jj++;
		}
	}

	// render FP_VISUALIZER_render_internal(): lines and dots in the visualizer plane
	FP_VISUALIZER_render_internal(now, mdna_atlas, r, scale);
}
