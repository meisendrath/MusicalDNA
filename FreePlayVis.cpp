#include "FreePlayVis.h"


#include "FP_VISUALIZER.h"
#include "FP_ROUTING.h"
#include "FPGameStatus.h"

#include "HAL.h"

#include "math.h"
#include <stdlib.h>


#define FREEPLAY_VERBOSE



#define M_2PI			(3.1415926535897932384626433 * 2.0)



extern HAL *gHAL;
extern FPGameStatus gGS;
extern bool makeSound;
extern double chord_id_time;
extern int note_number_recent[12];

extern float ADJUST_START_100;
extern float ADJUST_END_100;
extern float ADJUST_START_50;
extern float ADJUST_END_50;

extern double getcurrenttime();



// comment out this define to return to simple case: closest note is exactly what was last touched (or slid to)
//  as it stands, the closest note's movement would be limited to avoid causing any currently sustained notes to change octaves
//  this allows the user to build a chord, release, and strike all the notes at once without octave movement
#define NEW_WAY

// comment out this define to turn off the raw data visualization with the view
#define RAW_DATA_VISUALIZE


// new globals for this module
static double lastTouch = 0;
static TActiveTouch_Visualizer *pPrevTouch = NULL;

int			lastUsedOctave;
int			lastUsedNote;

bool		hitCenter;		// true if the user hit the center

bool		savedNotes[12];
bool		playingChord;

bool		lastUsedDir;

int			savedNote;
int			savedOctave;

// unmodified by "set the gap"
int			last2UsedNote;
int			last2UsedOctave;

int			saved2Note;
int			saved2Octave;

bool		enable_non_symmetric_gap;

int note_number_labels[12];
int note_number_mark[12];
int chord_detected = 0;
int octave_center = 0;
bool going_up = true;
bool recent_tritone = false;

int instrument_index_for_visualizer = 0;


#ifdef RAW_DATA_VISUALIZE
static void setup_note_number_labels() {
    int j;
    for (int i=-5; i<=5; i++) {
        j = (12 + i + lastUsedNote) % 12;
        note_number_labels[j] = i + lastUsedNote + 12*lastUsedOctave;
        note_number_mark[j] = 0;
    }
    j = (6 + lastUsedNote) % 12;
    note_number_mark[j] = 0;
    note_number_labels[j] = (-6) + lastUsedNote + 12*lastUsedOctave;
    //
    note_number_mark[(6 + lastUsedNote) % 12] = 1;
    //
    octave_center = lastUsedNote + 12*lastUsedOctave;
    //
    going_up = lastUsedDir;
}
#endif // RAW_DATA_VISUALIZE


void init_freeplay() {
    playingChord = false;
    FP_VISUALIZER_recent_notes_off();
    last2UsedOctave = lastUsedOctave = 5;
    last2UsedNote = lastUsedNote = 0;
    lastUsedDir = true;
#ifdef RAW_DATA_VISUALIZE
    setup_note_number_labels();
#endif // RAW_DATA_VISUALIZE
    enable_non_symmetric_gap = true;
}


#ifdef FREEPLAY_VERBOSE
static const char *get_mode_name(int flag) {
    switch (flag) {
        case VISUALIZER_NOTE:			return "VISUALIZER_NOTE";					break;
        case VISUALIZER_CENTER:			return "VISUALIZER_CENTER";					break;
        case VISUALIZER_OUTSIDE:		return "VISUALIZER_OUTSIDE";				break;
        case VISUALIZER_IMMEDIATE_TOGGLE:			return "VISUALIZER_IMMEDIATE_TOGGLE";				break;
        case VISUALIZER_DEFERRED_TOGGLE:			return "VISUALIZER_DEFERRED_TOGGLE";				break;
        case VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE:	return "VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE";		break;
        case VISUALIZER_ROTATE:			return "VISUALIZER_ROTATE";					break;
    }
    return "???";
}
#endif // FREEPLAY_VERBOSE



void				FreePlayVis::init() {
    mdna_atlas = TAtlas::get_cached_or_load("MDNA");
    // turn off any special effects on all orbs
    is_paused = false;
}
void				FreePlayVis::update(double now, float frame_time) {
}

void				FreePlayVis::rotateShape(int delta) {
    int deltaI = (int)fmaxf(delta,-12);
    if( deltaI < 0 )
        deltaI += 12;
    
    int newNotes[12];
    for( int i = 0; i < 12; i++ )
    {
        int note = note_number_recent[i];
        if( note )
        {
            //printf("   rotate: ");
            setNoteOff(note%12, note/12);
            
            note += delta;
            while( note >= 120 )
                note -= 12;
            while( note <= 0 ) // dont allow zero since that is considered not a note
                note += 12;
            
            newNotes[(i + deltaI) % 12] = note;
        }
        else
        {
            newNotes[(i + deltaI) % 12] = 0;
        }
    }
    
    for( int i = 0; i < 12; i++ )
    {
        int recent_note = 0;
        int newNote = newNotes[i];
        if( newNote ) {
            //printf("   rotate: ");
            setNoteOn(newNote%12, (int)(gGS.visualizer_volume*127), newNote/12, gGS.visualizer_art_index, gGS.visualizer_art_alpha);		// 120*base_channel_volume[0]???
            if( newNote > recent_note )
                recent_note = newNote;
        }
        
        note_number_recent[i] = recent_note;
        
    }
    
}

void				FreePlayVis::setNoteOn(int note, int volume, int nOctave, int art_selector, float art_alpha) {
#ifdef FREEPLAY_VERBOSE
printf("setNoteOn:%d vol:%d\n", note+(nOctave*12), volume);
#endif // FREEPLAY_VERBOSE
    ROUTING_note(ROUTING_FROM_TOUCH_INPUT, true, gGS.visualizer_channel, note+(nOctave*12), volume, art_selector, art_alpha);
    
    // if all notes are turned off - fade out chord id
    chord_id_time = getcurrenttime();
}

void				FreePlayVis::setNoteOff(int note, int nOctave) {
#ifdef FREEPLAY_VERBOSE
printf("setNoteOff:%d\n", note+(nOctave*12));
#endif // FREEPLAY_VERBOSE
    ROUTING_note(ROUTING_FROM_TOUCH_INPUT, false, gGS.visualizer_channel, note+(nOctave*12), 0);
    
    // if all notes are turned off - fade out chord id
    chord_id_time = getcurrenttime();
}


TActiveTouch		*FreePlayVis::touch(int phase, double now, TActiveTouch *t, bool is_inside) {
    // return a new TActiveTouch_Choice to "claim" the touch (and block further traversal)
    TActiveTouch *retval = NULL;
    float min_extent = fminf(extent[2], extent[3]);
    //	float radius = min_extent * 0.33f;
    float scale = min_extent / 300;
    float x = (t->cx - extent[0] - extent[2]/2)/scale;
    float y = (t->cy - extent[1] - extent[3]/2)/scale;
    float d = sqrtf(x*x + y*y);
    int touch_where;
    int touch_note = -1;
    float inner_radius = 100 - 35;		// after un-scaling
    float outer_radius = 100 + 35;
    if (d < inner_radius) { touch_where = VISUALIZER_CENTER; } else
        if (d > outer_radius) { touch_where = VISUALIZER_OUTSIDE; } else {
            touch_where = VISUALIZER_NOTE;
            touch_note = ((int)(6.5f-(atan2f(x, y)/(2*3.14159265f))*12)) % 12;
        }

#ifdef FREEPLAY_VERBOSE
printf(" >>> FreePlayVis::touch:%d at %g,%g at: %d %d", phase, x, y, touch_where, touch_note);
printf(" -- note_number_recent = [");
for (int i=0; i<12; i++)
    printf("%d ", note_number_recent[i]);
printf("]\n");
#endif // FREEPLAY_VERBOSE

    TActiveTouch_Visualizer *tt = dynamic_cast<TActiveTouch_Visualizer *>(t);

#ifdef FREEPLAY_VERBOSE
if (tt != NULL) printf("          flags=%s firstFlags=%s previousFlags=%s\n", get_mode_name(tt->flags), get_mode_name(tt->firstFlags), get_mode_name(tt->previousFlags));
#endif // FREEPLAY_VERBOSE


    switch (phase) {
        case 0:				// touchBegan
            if (is_inside) {
                tt = new TActiveTouch_Visualizer();
                tt->disambiguator = t->disambiguator;
                tt->owner = t->owner;
                tt->cx = t->cx;
                tt->cy = t->cy;
                tt->sx = t->sx;
                tt->sy = t->sy;
                tt->firstFlags = touch_where;
                tt->firstNote = touch_note;
                tt->flags = touch_where;
                tt->note = touch_note;
                tt->previousOctave  = lastUsedOctave;
                tt->previousNote	= tt->note;
                tt->previousFlags	= tt->flags;
                tt->octave = lastUsedOctave;
                tt->first_position_x = t->cx;
                tt->first_position_y = t->cy;
                t = tt;
                switch (tt->firstFlags) {
                    case VISUALIZER_NOTE:
                    {
                        bool holding_center = false;
                        bool holding_another_note = false;
                        for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                            TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                            if (ttt != NULL) {
                                if ((ttt->firstFlags == VISUALIZER_CENTER) && (ttt->flags == ttt->firstFlags)) holding_center = true;
                                if ((ttt->firstFlags == VISUALIZER_NOTE) && (ttt->flags == ttt->firstFlags)) holding_another_note = true;
                            }
                        }
                        if (holding_center) {
                            if (FP_VISUALIZER_isNoteActiveOnAnyChannel(tt->note) >= 0) {
                                tt->firstFlags = tt->flags = holding_center?VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE:VISUALIZER_DEFERRED_TOGGLE;
#ifdef FREEPLAY_VERBOSE
if (holding_center) printf("     -> VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE\n");
else printf("     -> VISUALIZER_DEFERRED_TOGGLE\n");
#endif // FREEPLAY_VERBOSE
                            } else if (!holding_another_note) {
#ifdef FREEPLAY_VERBOSE
printf("   immediate toggle on: ");
#endif // FREEPLAY_VERBOSE
                                setNoteOn(tt->note%12, (int)(gGS.visualizer_volume*127), tt->octave, gGS.visualizer_art_index, gGS.visualizer_art_alpha);		// 120*base_channel_volume[0]???
                                note_number_recent[tt->note] = tt->note%12 + 12*tt->octave;	// skip touches that are not touching a note
                                tt->firstFlags = tt->flags = holding_center?VISUALIZER_ROTATE:VISUALIZER_IMMEDIATE_TOGGLE;
#ifdef FREEPLAY_VERBOSE
if (holding_center) printf("     -> VISUALIZER_ROTATE\n"); else printf("     -> VISUALIZER_IMMEDIATE_TOGGLE\n");
#endif // FREEPLAY_VERBOSE
                            }
                        }
                        if (tt->firstFlags == VISUALIZER_NOTE) {
                            //    do noteTouchBegan:
                            // detect really recent notes
                            bool recentNoteInPast = false;
                            if ((now - lastTouch) < 0.025f) recentNoteInPast = true;			// *** why 0.025f?
                            
                            
                            lastTouch = now;
                            // if a recent note played, go back to the saved lastUsedNote & lastUsedOctave from before the previous note
                            if (recentNoteInPast) {
                                lastUsedNote = savedNote;											// go back to previous lastUsedNote & lastUsedOctave
                                lastUsedOctave = savedOctave;
                                last2UsedNote = saved2Note;
                                last2UsedOctave = saved2Octave;
                                setup_note_number_labels();
                                tt->octave = lastUsedOctave;
                                tt->dont_set_last_used = true;									// avoid future touchSlide changing lastUsedNote & lastUsedOctave
                                if (pPrevTouch != NULL) {
                                    pPrevTouch->dont_set_last_used = true;
                                }
#ifdef RAW_DATA_VISUALIZE
                                chord_detected = 1;
#endif // RAW_DATA_VISUALIZE

#ifdef FREEPLAY_VERBOSE
printf("     rept %d(%d) on %d %d\n", tt->note + (12 * lastUsedOctave), tt->flags, lastUsedNote, lastUsedOctave);
#endif // FREEPLAY_VERBOSE
                            } else {
                                savedNote = lastUsedNote;
                                savedOctave = lastUsedOctave;
                                saved2Note = last2UsedNote;
                                saved2Octave = last2UsedOctave;
                                tt->dont_set_last_used = false;
                                pPrevTouch = tt;
#ifdef RAW_DATA_VISUALIZE
                                chord_detected = 0;
#endif // RAW_DATA_VISUALIZE

#ifdef FREEPLAY_VERBOSE
printf("     note %d(%d) on %d %d\n", tt->note + (12 * lastUsedOctave), tt->flags, lastUsedNote, lastUsedOctave);
#endif // FREEPLAY_VERBOSE
                            }
                            tt->octave = lastUsedOctave;
                            // find nearest note/octave, determine if the octave needs to change
                            if (lastUsedNote < 6) {
                                if ((lastUsedNote+6) == tt->note) {				// tritone
#ifdef FREEPLAY_VERBOSE
printf("       octave-- at touch began\n");
#endif // FREEPLAY_VERBOSE
                                    if (tt->octave > 1) tt->octave--;
                                } else if ((lastUsedNote+6) < tt->note) {
#ifdef FREEPLAY_VERBOSE
printf("       octave-- at touch began\n");
#endif // FREEPLAY_VERBOSE
                                    if (tt->octave > 1) tt->octave--;
                                }
                            } else {
                                if ((lastUsedNote-6) == tt->note) {				// tritone
                                    //			if (tt->octave < 9) tt->octave++;
                                } else if ((lastUsedNote-6) > tt->note) {
                                    if (tt->octave < 9) tt->octave++;
#ifdef FREEPLAY_VERBOSE
printf("       octave++ at touch began\n");
#endif // FREEPLAY_VERBOSE
                                }
                            }
                            FP_VISUALIZER_recent_notes_off();
                            // play
#ifdef FREEPLAY_VERBOSE
printf("   play: ");
#endif // FREEPLAY_VERBOSE
                            setNoteOn(tt->note%12, (int)(gGS.visualizer_volume*127), tt->octave, gGS.visualizer_art_index, gGS.visualizer_art_alpha);		// 120*base_channel_volume[0]???
                            
                            
                            // prepare for next note: only update lastUsedNote & lastUsedOctave if no recent previous note
                            // advance the lastUsedNote/Octave, but don't make an already sounding note change octaves
                            int fromnote = lastUsedNote+12*lastUsedOctave;
                            int tonote = tt->note+12*tt->octave;
                            
                            // manage last direction
                            int from2note = last2UsedNote+12*last2UsedOctave;
                            //
                            recent_tritone = (6 == abs(tonote - from2note));
                            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                                TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                                if( ttt != NULL && ttt->firstFlags == VISUALIZER_NOTE ) for (int j=i+1; j<TActiveTouch::active_touches.size(); j++) {
                                    TActiveTouch_Visualizer *tttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[j]);
                                    if (tttt != NULL) if (6 == abs((tttt->note+12*tttt->octave) - (ttt->note+12*ttt->octave))) recent_tritone = true;
                                }
                            }
                            //
                            if (!tt->dont_set_last_used) {
                                if ((6 == abs(tonote - from2note)) || (0 == (tonote - from2note))) {		// it's a tritone or unison
                                    tonote = fromnote;					//  dont move the current lastUsedNote/Octave: it's confusing
                                } else {
                                    if (from2note < tonote) {
                                        lastUsedDir = true;
                                        if (!enable_non_symmetric_gap) tonote++;						// adjust to account for direction
                                        int minnote = 9999;
                                        for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                                            TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                                            if( ttt != NULL && ttt->firstFlags == VISUALIZER_NOTE ) {
                                                int nnn = ttt->note + 12*ttt->octave;
                                                if (minnote > nnn) minnote = nnn;
                                            }
                                        }
                                        minnote += 6;
                                        if (tonote > minnote) tonote = minnote;
                                    } else if (from2note > tonote) {
                                        lastUsedDir = false;
                                        int maxnote = -9999;
                                        for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                                            TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                                            if( ttt != NULL && ttt->firstFlags == VISUALIZER_NOTE ) {
                                                int nnn = ttt->note + 12*ttt->octave;
                                                if (maxnote < nnn) maxnote = nnn;
                                            }
                                        }
                                        maxnote -= 5;
                                        if (tonote < maxnote) tonote = maxnote;
                                    } else {
                                        if (lastUsedDir && !enable_non_symmetric_gap) tonote++;		// adjust to account for recent direction
                                    }
                                }
                                lastUsedNote = tonote % 12;
                                lastUsedOctave = tonote / 12;
                                //
                                last2UsedNote = tt->note;
                                last2UsedOctave = tt->octave;
                            }
                            //
                            setup_note_number_labels();
                        }
                    }
                        break;
                    case VISUALIZER_OUTSIDE:
#ifdef FREEPLAY_VERBOSE
printf("     touch outside: reset all notes\n");
#endif // FREEPLAY_VERBOSE
                        send("allnotesoff_by_touching_corner");
                        break;
                    case VISUALIZER_CENTER:
#ifdef FREEPLAY_VERBOSE
printf("     touch center: replay all notes\n");
#endif // FREEPLAY_VERBOSE
                        for (int i=0; i<12; i++) {
                            int n = note_number_recent[i];
                            if (n != 0) {
                                setNoteOn(n%12, (int)(gGS.visualizer_volume*127), n/12, gGS.visualizer_art_index, gGS.visualizer_art_alpha);		// 120*base_channel_volume[0]???
                            }
                        }
                        break;
                }
                retval = t;		// to claim the touch
            }
            break;
        case 1:				// touchMoved
        {
            tt->previousOctave  = tt->octave;
            tt->previousNote	= tt->note;
            tt->previousFlags	= tt->flags;
            if ((touch_where != VISUALIZER_NOTE) || (tt->previousNote != touch_note)) {
                tt->note			= touch_note;
                tt->flags			= touch_where;
                switch (tt->firstFlags) {
                    case VISUALIZER_NOTE:
                        switch (touch_where) {
                            case VISUALIZER_NOTE:
                            {
                                //   do noteTouchSlide:
#ifdef FREEPLAY_VERBOSE
printf("     slide note: %d octave: %d\n", tt->note, tt->octave );
#endif // FREEPLAY_VERBOSE
                                if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("     note %d(%d) off, ", tt->previousNote + (12 * tt->previousOctave), tt->previousFlags);
#endif // FREEPLAY_VERBOSE
                                    setNoteOff(tt->previousNote%12, tt->previousOctave);
                                }
                                
                                if (tt->note != -1) {
                                    
                                    if ((tt->previousNote != -1) && (tt->note != -1)) {
                                        
                                        // find nearest note/octave, determine if the octave needs to change
                                        if (tt->previousNote < 6) {
                                            if ((tt->previousNote+6) == tt->note) {				// tritone (not very likely in a slide...)
                                                if (tt->octave > 1) tt->octave--;
#ifdef FREEPLAY_VERBOSE
printf("       slide: octave--\n");
#endif // FREEPLAY_VERBOSE
                                            } else
                                                if ((tt->previousNote+6) < tt->note) {
                                                    if (tt->octave > 1) tt->octave--;
#ifdef FREEPLAY_VERBOSE
printf("       slide: octave--\n");
#endif // FREEPLAY_VERBOSE
                                                }
                                        } else {
                                            if ((tt->previousNote-6) == tt->note) {				// tritone (not very likely in a slide...)
#ifdef FREEPLAY_VERBOSE
printf("       SHOULD BE slide: octave++\n");
#endif // FREEPLAY_VERBOSE
                                            } else if ((tt->previousNote-6) > tt->note) {
                                                if (tt->octave < 9)
                                                    tt->octave++;
#ifdef FREEPLAY_VERBOSE
printf("       slide: octave++\n");
#endif // FREEPLAY_VERBOSE
                                            }
                                        }
                                    }
#ifdef FREEPLAY_VERBOSE
printf("     slide note %d(%d) ", tt->note + (12 * tt->octave), tt->flags);
#endif // FREEPLAY_VERBOSE
                                    setNoteOn(tt->note%12, (int)(gGS.visualizer_volume*127), tt->octave, gGS.visualizer_art_index, gGS.visualizer_art_alpha);		// 120*base_channel_volume[0]???
                                    // whole current state is the most recent
                                    
                                    // prepare for next note: only update lastUsedNote & lastUsedOctave if no recent previous note
                                    if (!tt->dont_set_last_used) {
                                        // advance the lastUsedNote/Octave, but don't make an already sounding note change octaves
                                        int fromnote = lastUsedNote+12*lastUsedOctave;
                                        int tonote = tt->note+12*tt->octave;
                                        
                                        // never even got here if the notes were the same, can't really slide a tritone => don't check for these conditions
                                        if (fromnote < tonote) {
                                            lastUsedDir = true;
                                            if (!enable_non_symmetric_gap) tonote++;						// adjust to account for direction
                                            int minnote = 9999;
                                            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                                                TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                                                if ((ttt != NULL) && (ttt->firstFlags == VISUALIZER_NOTE)) {
                                                    int nnn = ttt->note + 12*ttt->octave;
                                                    if (minnote > nnn) minnote = nnn;
                                                }
                                            }
                                            minnote += 6;
                                            if (tonote > minnote) tonote = minnote;
                                        } else if (fromnote > tonote) {
                                            lastUsedDir = false;
                                            int maxnote = -9999;
                                            for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                                                TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                                                if ((ttt != NULL) && (ttt->firstFlags == VISUALIZER_NOTE)) {
                                                    int nnn = ttt->note + 12*ttt->octave;
                                                    if (maxnote < nnn) maxnote = nnn;
                                                }
                                            }
                                            maxnote -= 5;
                                            if (tonote < maxnote) tonote = maxnote;
                                        } else {
                                            if (lastUsedDir && !enable_non_symmetric_gap) tonote++;		// adjust to account for recent direction
                                        }
                                        lastUsedNote = tonote % 12;
                                        lastUsedOctave = tonote / 12;
                                        //
                                        savedNote = lastUsedNote;
                                        savedOctave = lastUsedOctave;
                                        saved2Note = last2UsedNote;
                                        saved2Octave = last2UsedOctave;
                                        last2UsedNote = tt->note;
                                        last2UsedOctave = tt->octave;
                                        //
                                        setup_note_number_labels();
                                    }
                                }
                            }
                                break;
                            case VISUALIZER_OUTSIDE:
                                if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   slide outside: ");
#endif // FREEPLAY_VERBOSE
                                    setNoteOff(tt->previousNote%12, tt->previousOctave);
                                    tt->previousNote = -1;
                                }
                                break;
                            case VISUALIZER_CENTER:
                                if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   slide in center: ");
#endif // FREEPLAY_VERBOSE
                                    setNoteOff(tt->previousNote%12, tt->previousOctave);
                                    tt->previousNote = -1;
                                }
                                break;
                        }
                        break;
                    case VISUALIZER_OUTSIDE:
                        break;
                    case VISUALIZER_CENTER:
                        break;
                    case VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE:
                        tt->flags = VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE;
                        switch (touch_where) {
                            case VISUALIZER_NOTE:
#ifdef FREEPLAY_VERBOSE
printf("     do rotate -> VISUALIZER_ROTATE\n");
#endif // FREEPLAY_VERBOSE
                                //   rotate
                                if (tt->note != -1) {
                                    int prevOctave = tt->octave;
                                    if (tt->previousNote < 6) {
                                        if ((tt->previousNote+6) < tt->note) {
                                            if (tt->octave > 1) tt->octave--;
#ifdef FREEPLAY_VERBOSE
printf("       octave--\n");
#endif // FREEPLAY_VERBOSE
                                        }	
                                    } else {
                                        if ((tt->previousNote-6) > tt->note) {
                                            if (tt->octave < 9) tt->octave++;
#ifdef FREEPLAY_VERBOSE
printf("       octave++\n");
#endif // FREEPLAY_VERBOSE
                                        }
                                    }
                                    
                                    int delta = (tt->note + 12*tt->octave) - (tt->previousNote + 12*prevOctave);
                                    if (delta != 0) {
                                        if( delta == 11 )
                                            delta -= 12;
                                        rotateShape(delta);
                                        
                                        lastUsedOctave = tt->octave;
                                        lastUsedNote = tt->note % 12;
                                        
                                    }
                                }
                                tt->firstFlags = tt->flags = VISUALIZER_ROTATE;
                                break;
                            case VISUALIZER_OUTSIDE:
                                break;
                            case VISUALIZER_CENTER:
                                break;
                        }
                        break;
                    case VISUALIZER_ROTATE:
                        tt->flags = VISUALIZER_ROTATE;
                        switch (touch_where) {
                            case VISUALIZER_NOTE:
#ifdef FREEPLAY_VERBOSE
printf("     do rotate\n");
#endif // FREEPLAY_VERBOSE
                                //   rotate
                                if (tt->note != -1) {
                                    int prevOctave = tt->octave;
                                    if (tt->previousNote < 6) {
                                        if ((tt->previousNote+6) < tt->note) {
                                            if (tt->octave > 1) tt->octave--;
#ifdef FREEPLAY_VERBOSE
printf("       octave--\n");
#endif // FREEPLAY_VERBOSE
                                        }	
                                    } else {
                                        if ((tt->previousNote-6) > tt->note) {
                                            if (tt->octave < 9) tt->octave++;
#ifdef FREEPLAY_VERBOSE
printf("       octave++\n");
#endif // FREEPLAY_VERBOSE
                                        }
                                    }
                                    
                                    int delta = (tt->note + 12*tt->octave) - (tt->previousNote + 12*prevOctave);
                                    if (delta != 0) {
                                        if( delta == 11 )
                                            delta -= 12;
                                        rotateShape(delta);
                                        
                                        lastUsedOctave = tt->octave;
                                        lastUsedNote = tt->note % 12;
                                        
                                    }
                                }
                                break;
                            case VISUALIZER_OUTSIDE:
                                break;
                            case VISUALIZER_CENTER:
                                break;
                        }
                        break;
                }
            }
        }
            break;
        case 2:				// touchEnded
        {
            tt->previousOctave  = tt->octave;
            tt->previousNote	= tt->note;
            tt->previousFlags	= tt->flags;
            switch (tt->firstFlags) {
                case VISUALIZER_NOTE:
#ifdef FREEPLAY_VERBOSE
printf(" (touch_where=%d) ", touch_where);
#endif // FREEPLAY_VERBOSE
                    switch (touch_where) {
                        case VISUALIZER_NOTE:
                            if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   note released: ");
#endif // FREEPLAY_VERBOSE
                                setNoteOff(tt->previousNote%12, tt->previousOctave);
                                note_number_recent[tt->previousNote] = tt->previousNote%12 + 12*tt->previousOctave;	// skip touches that are not touching a note
#ifdef FREEPLAY_VERBOSE
printf("   note_number_recent = [");
for (int i=0; i<12; i++)
    printf("%d ", note_number_recent[i]);
printf("] lastUsed=%d/%d\n", lastUsedNote, lastUsedOctave);
#endif // FREEPLAY_VERBOSE
                                tt->previousNote = -1;
                            }
#ifdef FREEPLAY_VERBOSE
else printf("previousNote == -1!!! ");
#endif // FREEPLAY_VERBOSE
                            break;
                        case VISUALIZER_OUTSIDE:
#ifdef FREEPLAY_VERBOSE
printf("   note released o: ");
#endif // FREEPLAY_VERBOSE
                            if (tt->previousNote != -1) setNoteOff(tt->previousNote%12, tt->previousOctave);
                            break;
                        case VISUALIZER_CENTER:
#ifdef FREEPLAY_VERBOSE
printf("   note released c: ");
#endif // FREEPLAY_VERBOSE
                            if (tt->previousNote != -1) setNoteOff(tt->previousNote%12, tt->previousOctave);
                            break;
                    }
                    break;
                case VISUALIZER_OUTSIDE:
                    break;
                case VISUALIZER_CENTER:
                    FP_VISUALIZER_allgviznotesoff();
#ifdef FREEPLAY_VERBOSE
printf("   center released: ");
#endif // FREEPLAY_VERBOSE
                    break;
                case VISUALIZER_IMMEDIATE_TOGGLE:
                    if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   toggle i released: ");
#endif // FREEPLAY_VERBOSE
                        setNoteOff(tt->previousNote%12, tt->previousOctave);
                        tt->previousNote = -1;
                    }
                    break;
                case VISUALIZER_DEFERRED_TOGGLE:
                    if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   toggle d released: ");
#endif // FREEPLAY_VERBOSE
                        setNoteOff(tt->previousNote%12, tt->previousOctave);
                        note_number_recent[tt->previousNote%12] = 0;
                        tt->previousNote = -1;
                    }
                    break;
                case VISUALIZER_DEFERRED_TOGGLE_OR_ROTATE:
                    if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
#endif // FREEPLAY_VERBOSE
                        setNoteOff(tt->previousNote%12, tt->previousOctave);
                        note_number_recent[tt->previousNote%12] = 0;
                        tt->previousNote = -1;
                    }
                    break;
                case VISUALIZER_ROTATE:
                {
                    bool holding_center = false;
                    bool holding_another_note = false;
                    for (int i=0; i<TActiveTouch::active_touches.size(); i++) {
                        TActiveTouch_Visualizer *ttt = dynamic_cast<TActiveTouch_Visualizer *>(TActiveTouch::active_touches[i]);
                        if (ttt != NULL) {
                            if ((ttt->firstFlags == VISUALIZER_CENTER) && (ttt->flags == ttt->firstFlags)) holding_center = true;
                            if ((ttt->firstFlags == VISUALIZER_NOTE) && (ttt->flags == ttt->firstFlags)) holding_another_note = true;
                        }
                    }
                    
                    if (!holding_another_note && !holding_center) {
                        FP_VISUALIZER_allgviznotesoff();
#ifdef FREEPLAY_VERBOSE
printf("   center released: ");
#endif // FREEPLAY_VERBOSE
                    }
                    
                }
                    break;
            }
            if (tt == pPrevTouch) pPrevTouch = NULL;
        }
            break;
        case 3:				// touchCancelled
        {
            if (tt->previousNote != -1) {
#ifdef FREEPLAY_VERBOSE
printf("   cancelled: ");
#endif // FREEPLAY_VERBOSE
                setNoteOff(tt->previousNote%12, tt->previousOctave);
                tt->previousNote = -1;
            }
            if (tt == pPrevTouch) pPrevTouch = NULL;
        }
            break;
    }
#ifdef FREEPLAY_VERBOSE
printf(" <<< FreePlayVis\n");
#endif // FREEPLAY_VERBOSE
    return retval;
}



void				FreePlayVis::render(double now) {
    gHAL->matrix_push();
    gHAL->matrix_translate(extent[0]+extent[2]/2, extent[1]+extent[3]/2, 0);
    
    float min_extent = fminf(extent[2], extent[3]);
    float radius = min_extent * 0.33f;
    float scale = min_extent / 400;
    FP_VISUALIZER_render(mdna_atlas, extent[0]+extent[2]/2, extent[1]+extent[3]/2, min_extent, radius, true, scale);
    gHAL->matrix_pop();
}

void FreePlayVis::cleanup_on_exit() {
    for (int nt=0; nt<12; nt++) {
        note_number_recent[nt]=0;
        // also turn off all special effects on all note orbs
    }
}

bool FreePlayVis::message(const std::string &msg, float arg1, float arg2) {
    bool retval = false;
    
    if (strcmp("allnotesoff", msg.c_str()) == 0) {
        cleanup_on_exit();
        return true;		// accept the message
    }
    
    if (strcmp("allnotesoff_by_touching_corner", msg.c_str()) == 0) {
        cleanup_on_exit();
        return true;		// accept the message
    }
    
    
    if ((strcmp("note_particle_control", msg.c_str()) == 0)) {
        
        if (arg2 == 0) {
            // turn off any special effects on the orb at ((int)arg1 % 12)
        } else {
            // turn on any desired special effects on the orb at ((int)arg1 % 12)
        }
        return true;		// accept the message
    }
    
    if (strcmp("freeplay_starting", msg.c_str()) == 0) {
//        gGS.PVar->setVar("game_select", GAME_SELECT_FREE_PLAY);
        return true;
    }
    
    //
    printf("FreePlayVis MESSAGE IGNORED: %s\n", msg.c_str());
    return retval;
}


