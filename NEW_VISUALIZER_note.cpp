// NEW_VISUALIZER_NOTE

// This function is called whenever there is a MIDI Note On or MIDI Note Off event to be processed, whether from an actual instrument or from a MIDI file. It is also called when a note ‘orb’ is touched or released. This simply populates gViz – which is then used by the various rendering functions to determine what lines & dots to draw at any given point in time. Each track in gViz keeps track of whether it is sounding a note at any of the twelve orbs, when each orb was most recently turned on, whether it has sounded a note at any of the twelve orbs, and when each orb was most recently turned off. It also stores its assigned instrument patch (from the soundfont), art index, and art transparency.

// OTHER FUNCTIONS handle the actual playing of the MIDI sounds. This little routine is the link between the audio and visual aspects of the game.


void NEW_VISUALIZER_note(int ch, int note, int vel, int track, int art_index, float art_alpha, bool noteOn) {

  if (gViz.size() < track+1) 
    gViz.resize(track+1);
  gViz[track].note_active[note % 12] = noteOn?note:0;
  gViz[track].note_was_active[note % 12] = note;
  gViz[track].instrument_index = NEW_SYNTH_get_patch(ch);
  gViz[track].art_index = art_index;
  gViz[track].art_alpha = art_alpha;
  if (noteOn) gViz[track].note_timestamp[note % 12] = getcurrenttime();
    else gViz[track].note_end_timestamp[note % 12] = getcurrenttime();
}