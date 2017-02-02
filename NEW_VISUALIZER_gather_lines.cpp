


//  NEW_VISUALIZER_gather_lines

// The calling function loops through this once for each track in gViz * the 11 possible lines lengths.
// (gViz is populated and updated by NEW_VISUALIZER_note. It contains the status of which note “orbs” are active at any given time on any given track.)

static void NEW_VISUALIZER_gather_lines(int linelen, int track, float scale) {
  double now = getcurrenttime();
  for (int idx=0; idx<12; idx++) {
    int idx2a = idx+linelen;
    int idx2 = idx2a % 12;
    int idx1 = idx;
    if ((gViz[track].note_active[idx1] == 0) && (gViz[track].note_active[idx2] == 0)) 
      continue;  // skip out if both note orbs are not active (i.e. both are silent)

//---------------------
// one or both notes being tested are still sounding, create an ActiveLine with the two notes as “endcaps”
    ActiveLine newline;
    newline.track = track;
    newline.instrument_index = gViz[track].instrument_index;
    newline.art_index = gViz[track].art_index;
    newline.art_alpha = gViz[track].art_alpha;
    newline.note1 = idx1;  // the “orbs” – values 0..11
    newline.note2 = idx2;
    newline.notenum1 = gViz[track].note_was_active[idx1];  // the MIDI note value (e.g. 60 for middle C)
    newline.notenum2 = gViz[track].note_was_active[idx2];
    newline.line_length = linelen;
    newline.start_time = newline.last_time = now;
    newline.is_melody = 0;
    newline.done_melody = 0;
    newline.orphan = 0;
    
    bool bFound = false;
    if (gViz[track].note_active[idx2] == 0) {          
      // second note is inactive (silent), swap them
      // melody lines wouldn’t be moving from an on note to an off note
      // and we only get here if at least one of the two endcap notes is active (sounding)
      int tmp = idx1;
      idx1 = idx2;
      idx2 = tmp;
      newline.note1 = idx2;
      newline.note2 = idx1;
      newline.notenum1 = gViz[track].note_was_active[idx2];
      newline.notenum2 = gViz[track].note_was_active[idx1];
    }
    

    if (gViz[track].note_active[idx1] == 0) {  
      // only one endcap note is active (sounding), so this must be a melody line or no line at all

      int most_recent_release;
      // bail if this is not the most recent release (note off) for the “silent” endcap’s orb
      double t = gViz[track].note_timestamp[idx1];
      most_recent_release = idx1;
      for (int j=0; j<12; j++) {
        if ((j != idx1) && (j != idx2)) {
          if (t < gViz[track].note_timestamp[j]) {
            most_recent_release = j;
          }
        }
      }
      if (idx1 != most_recent_release) 
        continue;  
//---------------------------------------------------------------------


// ONLY ADD A LINE TO THE MOST RECENT NOTE BEFORE idx2
// if we can find a note that’s between idx1 and idx2, bail
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
      if (idx1 != most_recent_release) 
        continue;


// ------------------------------------
//
// We only get here for any pair of note “orbs” if all of the following are true:
//    only one of the endcap orbs is associated with a MIDI note that is still sounding
//    the silent endcap orb hasn’t been “silenced” again between ‘now’ and the endcap note event
//    there are no notes that have started between the timestamps of the two endcap notes
//
// if the first endcap note ended and the second endcap note started within the last half-second:
//
// (human fudge factor – you may be intending to play a series of individual melody notes, but if you’re
// playing ‘legato’, there will almost certainly be some overlap between the first note ending and the
// second one starting – this is, theoretically, to avoid flashing a brief chord line in such instances
// we’re not completely happy with the way this works in practice)
//
// check for a line in m_ActiveLines connecting the same endcap orbs – 
// if a chord line is found, change it to a melody line 
// if no line is found, add the current line to m_ActiveLines 

      if ((fabsf((float)(now - gViz[track].note_end_timestamp[idx1])) < 0.5f) 
           && (fabsf((float)(now - gViz[track].note_timestamp[idx2])) < 0.5f)) {    
        // find and convert possible chord line mistakenly connecting these two notes
        for( int i = 0; i < m_ActiveLines.size(); i++ )
        {
          ActiveLine& line = m_ActiveLines[i];
          if( line.track == track && line.is_melody == 0 && line.orphan == 0 
             && ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
          {
            // found a matching chord line
            bFound = true;
            newline.is_melody = 1;
            m_ActiveLines[i] = newline;    // replace with melody line
            break;
          }

          if( line.track == track && line.is_melody == 1 && line.orphan == 0 
             && ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
          {
            // found a matching melody line
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
          NEW_VISUALIZER_add_line(newline);
        }
      }
    } else  

// we only get here if both endcap notes are still sounding, so we’re dealing with a chord line
// check m_ActiveLines to see if a chord line with the same endcap orbs is already there
// if so, update the timestamp to show the last time this chord line was “active”
// if not, add the current line to m_ActiveLines
    {                          
      for( int i = 0; i < m_ActiveLines.size(); i++ )
      {
        ActiveLine& line = m_ActiveLines[i];
        if( line.track == track && line.is_melody == 0 && 
          ((line.note1 == idx1 && line.note2 == idx2) || (line.note1 == idx2 && line.note2 == idx1)))
        {
          // found a matching chord line - update the time stamp
          bFound = true;
          line.last_time = getcurrenttime();
          break;
        }
        
      }
      if( !bFound )
      {
        // line not found - add it to the list
        NEW_VISUALIZER_add_line(newline);
      }
    }
  }
}