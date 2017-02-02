// trackcsv  (with two associated enum declarations so that you have the actual MIDI codes we are using)

// This function contains the main functionality for reading in a MIDI file and populating gRecordedSong with the note events (and other events) that we will use in the game. These MIDI files contain our own additional commands – beyond the MIDI standard. I’ll provide a separate file with an overview of what additions we’ve made. Be forewarned: in a PART track, the 'channel' value for any given event is used to know what NON-PART TRACK to affect.



typedef enum {
  /* Channel voice messages */
  NoteOff = 0x80,
  NoteOn = 0x90,
  PolyphonicKeyPressure = 0xA0,
  ControlChange = 0xB0,
  ProgramChange = 0xC0,
  ChannelPressure = 0xD0,
  PitchBend = 0xE0,
  
  /* Channel mode messages */
  ChannelMode = 0xB8,
  
  /* System messages */
  SystemExclusive = 0xF0,
  SystemCommon = 0xF0,
  SystemExclusivePacket = 0xF7,
  SystemRealTime = 0xF8,
  SystemStartCurrentSequence = 0xFA,
  SystemContinueCurrentSequence = 0xFB,
  SystemStop = 0xFC,
  
  /* MIDI file-only messages */
  FileMetaEvent = 0xFF
} midi_command;



typedef enum {
  SequenceNumberMetaEvent = 0,
  TextMetaEvent = 1,
  CopyrightMetaEvent = 2,
  TrackTitleMetaEvent = 3,
  TrackInstrumentNameMetaEvent = 4,
  LyricMetaEvent = 5,
  MarkerMetaEvent = 6,
  CuePointMetaEvent = 7,
  
  ChannelPrefixMetaEvent = 0x20,
  PortMetaEvent = 0x21,
  EndTrackMetaEvent = 0x2F,
  
  SetTempoMetaEvent = 0x51,
  SMPTEOffsetMetaEvent = 0x54,
  TimeSignatureMetaEvent = 0x58,
  KeySignatureMetaEvent = 0x59,
  
  SequencerSpecificMetaEvent = 0x7F
} midifile_meta_event;



static void trackcsv(byte *trk, long trklen, const int ppq) {
  int levt = 0;
  int evt, note, vel, control, value, type;
  char channel;
  vlint len;
  byte *titem;
  vlint abstime = 0;              /* Absolute time in track */
  bool sawNotes = false;
  

  gRecordedSong->MIDIparser_is_part = 0;    // UNKNOWN-PART
  TRecordedTrack *cur_track = new TRecordedTrack();
  cur_track->name = "noname";
  cur_track->channel_num = (int)gRecordedSong->tracks.size();
  cur_track->cur_event_idx = 0;
  cur_track->ffn_event_idx = 0;
  cur_track->loop = 0;
  cur_track->is_part = 0;
  cur_track->pitch_bend = 0;
  cur_track->controlBits = 0;
  cur_track->last_play_time = -20;

// fields without "i_" are controlled by the referencing PART track
  cur_track->mixer_volume = 1.0f;
  cur_track->i_instrument = cur_track->instrument = 0;
  cur_track->i_running_volume = cur_track->running_volume = 1.0f;
  cur_track->i_master_enable = cur_track->master_enable = 0;    // default: a channel is NOT a game track
  cur_track->i_muteMode = cur_track->muteMode = 0;      // not muted (unless the name starts with "part:")
  if (gGS.PVar->getVar("game_select")->asInt() == GAME_SELECT_PREVIEW) // as opposed to CF game play
    cur_track->i_muteViz = cur_track->muteViz = 0;
  else 
    cur_track->i_muteViz = cur_track->muteViz = 1;
  cur_track->i_art_selector = cur_track->art_selector = 0;
  cur_track->i_art_alpha = cur_track->art_alpha = 1.0f;
  cur_track->i_drums_enable = cur_track->drums_enable = false;
  
  gRecordedSong->tracks.push_back(cur_track);

  while (trklen > 0) {
    vlint tlapse = vlength(&trk, &trklen);
    abstime += tlapse;

    /* Handle running status; if the next byte is a data byte, reuse the last command seen in the track. */
    if (*trk & 0x80) {
      evt = *trk++;
      
      /* One subtlety: we only save channel voice messages for running status.  System messages and file
       meta-events (all of which are in the 0xF0-0xFF range) are not saved, as it is possible to carry a
       running status across them.  You may have never seen this done in a MIDI file, it’s happened. */
      
      if ((evt & 0xF0) != 0xF0) levt = evt;
      trklen--;
    } else {
      evt = levt;
    }
    
    channel = evt & 0xF;    
    /* Channel messages */
    switch (evt & 0xF0) {
      case NoteOff:     /* Note off */
        if (trklen < 2) return;
        trklen -= 2;
        note = *trk++;
        vel = *trk++;
        if (gRecordedSong->MIDIparser_is_part == 0) 
          gRecordedSong->MIDIparser_is_part = -1;
        if (gRecordedSong->MIDIparser_is_part == 0) {
          printf(" *** ERROR *** NoteOff: unexpected n%d v%d\n", note, vel);
        } else {  
          TRecordedEvent ev;
          ev.eventType = 2;
          ev.mark_any = 0;
          ev.channel = channel;
          ev.note_hit = 0;
          ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
          ev.xnoteNum = note;
          ev.velocity = vel;
          if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
            gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
          if (gRecordedSong->MIDIparser_is_part == +1) {
            if ((gRecordedSong->tracks.size() <= channel) || 
                (gRecordedSong->tracks[channel]->muteMode != 0)) {
              printf(" *** ERROR *** NoteOff: PART channel(%d) must refer to a NON-PART track n%d v%d\n",
                           channel, note, vel);
            } else 
              cur_track->events.push_back(ev);  
          } else {
            if (channel != cur_track->channel_num) {
              ev.channel = (char)cur_track->channel_num;
              cur_track->events.push_back(ev);
            } else 
              cur_track->events.push_back(ev);  
          }
        }
        continue;
      case NoteOn:     /* Note on */
        if (trklen < 2) return;
        trklen -= 2;
        note = *trk++;
        vel = *trk++;     /*  A note on with a velocity of 0 is actually a note off */
        sawNotes = true;
        if (gRecordedSong->MIDIparser_is_part == 0) 
          gRecordedSong->MIDIparser_is_part = -1;
        if (gRecordedSong->MIDIparser_is_part == 0) 
          printf(" *** ERROR *** NoteOn: unexpected n%d v%d\n", note, vel); 
        else {    
          TRecordedEvent ev;
          ev.eventType = vel==0?2:1;
          ev.mark_any = 0;
          ev.channel = channel;
          ev.note_hit = 0;
          ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
          ev.xnoteNum = note;
          ev.velocity = vel;
          if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
            gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
          if (gRecordedSong->MIDIparser_is_part == +1) {
            if ((gRecordedSong->tracks.size() <= channel) || 
                (gRecordedSong->tracks[channel]->muteMode != 0)) {
              printf(" *** ERROR *** NoteOn: PART channel(%d) must refer to a NON-PART track n%d v%d\n",
                       channel, note, vel);
            } else 
              cur_track->events.push_back(ev);  
          } else {
            if (channel != cur_track->channel_num) {
              ev.channel = (char)cur_track->channel_num;
              cur_track->events.push_back(ev);
            } else 
              cur_track->events.push_back(ev);  
          }
        }
        continue;
      case PolyphonicKeyPressure: /* Aftertouch */
        if (trklen < 2) return;
        trklen -= 2;
        note = *trk++;
        vel = *trk++;
        continue;
      case ControlChange:  /* Control change */
        if (trklen < 2) return;
        trklen -= 2;
        control = *trk++;
        value = *trk++;

        if (gRecordedSong->MIDIparser_is_part == 0) 
          gRecordedSong->MIDIparser_is_part = -1;
        if (gRecordedSong->MIDIparser_is_part == 0) 
          printf(“ERROR ControlChange: unexpected c%d v%d\n", control, value); 
        else {    
          if (gRecordedSong->MIDIparser_is_part == +1) {
            if ((gRecordedSong->tracks.size() <= channel) || 
                (gRecordedSong->tracks[channel]->muteMode != 0)) {
              if ((channel == cur_track->channel_num) && (control == 7) && (value == 0) && (!sawNotes)) {
                printf(" *** NOT AN ERROR *** set volume=0 for PART tracks before any notes is recommended");
              } else {
                printf(" *** ERROR *** ControlChange: PART channel(%d) must refer to a NON-PART track n%d v%d",
                       channel, note, vel);
              }
            } else {
              TRecordedEvent ev;
              ev.eventType = 3;
              ev.mark_any = 0;
              ev.channel = channel;
              ev.note_hit = 0;
              ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
              ev.xnoteNum = control;
              ev.velocity = value;
              if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
                gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
              cur_track->events.push_back(ev);  
            }
          } else {
            if (channel != cur_track->channel_num) {
              printf(" *** ERROR *** ControlChange: NON-PART channel(%d) must match track number(%d) n%d v%d\n",
                      channel, cur_track->channel_num, note, vel);
            } else {
              bool save_event = true;
              switch (control) {
                case 18:      //CFMIDI: ControlChange: NON-PART: #18: set <THIS> track instrument
                  if (!sawNotes)        
                    cur_track->i_instrument = cur_track->instrument = value;
                  break;
                case 7: case 19:  //CFMIDI: ControlChange: NON-PART: #7 or #19: set <THIS> track volume
                  if (!sawNotes)        
                    cur_track->i_running_volume = cur_track->running_volume = value / 127.0f;
                  break;
                case 20:      //CFMIDI: ControlChange: NON-PART: #20: set mute
                  if (!sawNotes)        
                    cur_track->i_muteMode = cur_track->muteMode = (char)value;
                  break;
                case 21:      //CFMIDI: ControlChange: NON-PART: #21: set master_enable
                  if (!sawNotes)        
                    cur_track->i_master_enable = cur_track->master_enable = (char)value;
                  break;
                case 22:      //CFMIDI: ControlChange: NON-PART: #22: set muteViz
                  if (!sawNotes)        
                    cur_track->i_muteViz = cur_track->muteViz = (char)value;
                  break;
                case 23:      //CFMIDI: ControlChange: NON-PART: #23: set art_selector
                  if (!sawNotes)        
                    cur_track->i_art_selector = cur_track->art_selector = value;
                  break;
                case 24:      //CFMIDI: ControlChange: NON-PART: #24: set art_alpha
                  if (!sawNotes)        
                    cur_track->i_art_alpha = cur_track->art_alpha = value / 127.0f;
                  break;
                case 25:      //CFMIDI: ControlChange: NON-PART: #25: set drums_enable
                  if (!sawNotes)        
                    cur_track->i_drums_enable = cur_track->drums_enable = value;
                  break;
                case 28:      //CFMIDI: ControlChange: NON-PART: #28: interrupt melody line
                case 29:      //CFMIDI: ControlChange: NON-PART: #29: join melody line
                case 30:      //CFMIDI: ControlChange: NON-PART: #30: local enable/disable
                  break;
                default:
                  printf(" *** ERROR *** ControlChange: NON-PART illegal c%d v%d\n", control, value);
                  save_event = false;
                  break;
              }
              if (save_event) {
                TRecordedEvent ev;
                ev.eventType = 3;
                ev.mark_any = 0;
                ev.channel = channel;
                ev.note_hit = 0;
                ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
                ev.xnoteNum = control;
                ev.velocity = value;
                if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
                  gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
                cur_track->events.push_back(ev);  
              }
            }
          }
        }
        continue;
      case ProgramChange:  /* Program change */
        if (trklen < 1) return;
        trklen--;
        note = *trk++;
        if (gRecordedSong->MIDIparser_is_part == 0)  
          gRecordedSong->MIDIparser_is_part = -1;
        if (gRecordedSong->MIDIparser_is_part == 0) 
          printf(" *** ERROR *** ProgramChange: unexpected c%d v%d\n", control, value); 
        else {    
          if (gRecordedSong->MIDIparser_is_part == +1) {
            if ((gRecordedSong->tracks.size() <= channel) || 
                (gRecordedSong->tracks[channel]->muteMode != 0)) {
              printf(" *** ERROR *** ProgramChange: PART channel(%d) must refer to a NON-PART track n%d v%d\n",
                      channel, note, vel);  
            } else {
              TRecordedEvent ev;
              ev.eventType = 3;
              ev.mark_any = 0;
              ev.channel = channel;
              ev.note_hit = 0;
              ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
              ev.xnoteNum = 1018;
              ev.velocity = note;
              if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
                gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
              cur_track->events.push_back(ev);  
            }
          } else {
            if (channel != cur_track->channel_num) {
              printf(" *** ERROR *** ProgramChange: NON-PART channel(%d) must match track number(%d) n%d v%d\n",
                      channel, cur_track->channel_num, note, vel); 
            } else {
              cur_track->instrument = note;    
              TRecordedEvent ev;
              ev.eventType = 3;
              ev.mark_any = 0;
              ev.channel = channel;
              ev.note_hit = 0;
              ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
              ev.xnoteNum = 1018;
              ev.velocity = note;
              if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
                gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
              cur_track->events.push_back(ev);
            }
          }
        }
        continue;
      case ChannelPressure: /* Channel pressure (aftertouch) */
        if (trklen < 1) return;
        trklen--;
        vel = *trk++;
        continue;
      case PitchBend:     /* Pitch bend */
        if (trklen < 2) return;
        trklen -= 2;
        value = *trk++;
        value = value | ((*trk++) << 7);
        {
          TRecordedEvent ev;
          ev.eventType = 4;
          ev.mark_any = 0;
          ev.channel = channel;
          ev.note_hit = 0;
          ev.timeStamp = abstime / (ppq / gRecordedSong->MIDIparser_sec_per_beat);
          ev.xnoteNum = control;
          ev.velocity = value-8192;
          if (gRecordedSong->MIDIparser_playback_length < ev.timeStamp) 
            gRecordedSong->MIDIparser_playback_length = ev.timeStamp;
          cur_track->events.push_back(ev);  
          cur_track->pitch_bend = value-8192;
        }
        continue;
      default:
        break;
    }
    
    /* System exclusive messages */
    switch (evt) {
      case SystemExclusive:
      case SystemExclusivePacket:
        len = vlength(&trk, &trklen);
        break;
      case FileMetaEvent:        /* File meta-events */
        if (trklen < 2) return;
        trklen -= 2;
        type = *trk++;
        len = vlength(&trk, &trklen);
        titem = trk;
        trk += len;
        trklen -= len;
        
        switch (type) {
          case SequenceNumberMetaEvent:
            break;
            
          case TextMetaEvent:
            break;
            
          case CopyrightMetaEvent:
            break;
            
          case TrackTitleMetaEvent:
            if (gRecordedSong->MIDIparser_is_part != 0) 
              printf(" *** ERROR *** TrackTitleMetaEvent: unexpected\n"); 
            else {
              std::string s;
              for (int i=0; i<len; i++) 
                s += titem[i];
              cur_track->name = s.c_str();
              if ((strncmp("part:",s.c_str(),5)==0) || (strncmp("Part:",s.c_str(),5)==0) ||
                   (strncmp("PART:",s.c_str(),5)==0)) {
                gRecordedSong->MIDIparser_is_part = +1;
                cur_track->i_muteMode = cur_track->muteMode = 1;
                cur_track->i_muteViz = cur_track->muteViz = 1;
                cur_track->is_part = true;
              } else {
                gRecordedSong->MIDIparser_is_part = -1;
              }
            }
            break;
            
          case TrackInstrumentNameMetaEvent:
            break;
            
          case LyricMetaEvent:
            break;
            
          case MarkerMetaEvent:
            break;
            
          case CuePointMetaEvent:
            break;
            
          case ChannelPrefixMetaEvent:
            break;
            
          case PortMetaEvent:
            break;
            
          case EndTrackMetaEvent:
            trklen = -1;
            break;
            
          case SetTempoMetaEvent:
            if (abstime == 0) {
              gRecordedSong->MIDIparser_sec_per_beat = 
                    ((titem[0] << 16) | (titem[1] << 8) | titem[2]) / 1000000.0f;
            }
            break;
            
          case SMPTEOffsetMetaEvent:
            break;
            
          case TimeSignatureMetaEvent:
            if (abstime == 0) {
              gRecordedSong->MIDIparser_quarter_notes_per_measure = titem[0];
            }
            break;
            
          case KeySignatureMetaEvent:
            break;
            
          case SequencerSpecificMetaEvent:
            break;
            
          default:
            printf(" *** ERROR *** Unknown meta event type 0x%02X, %ld bytes of data.\n", type, len);
            printf("Unknown_meta_event, %d, %lu", type, len); 
            for (int i = 0; i < len; i++) 
              printf(", %d", titem[i]); printf("\n");
            break;
        }
        break;
        
      default:
        printf(" *** ERROR *** Unknown event type 0x%02X.\n", evt);
        printf("Unknown_event, %02Xx\n", evt);
        break;
    }
  }

  bool done = false;
// sorting loop
  while (!done) {
    done = true;
    for (int i=0; (i+1)<cur_track->events.size(); i++) {
      TRecordedEvent *ev1 = &cur_track->events[i];
      for (int j=i+1; j<cur_track->events.size(); j++) {
        TRecordedEvent *ev2 = &cur_track->events[j];
        if (ev1->timeStamp != ev2->timeStamp) break;
        if ((ev1->eventType == 1) && (ev2->eventType == 2) && (ev1->xnoteNum == ev2->xnoteNum)) {
          TRecordedEvent e = cur_track->events[i];
          cur_track->events[i] = cur_track->events[j];
          cur_track->events[j] = e;
          done = false;
        }
      }
    }
  }
}
