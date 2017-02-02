2 
When a song is started (whether in preview, practice, or play mode), the MIDI file is parsed and loaded to memory (into gRecordedSong), the playback point is set to < 0, instrument patches are loaded into the synthesizer, art work is loaded to memory, the MP3 file is loaded to memory, and the MP3 player is initialized.
(See trackcsv)

As time passes while a song is playing (either MIDI-only or a combination of MP3 & MIDI). A standard MP3 player controls the playback of that media. At each frame (30fps), update & render are called. Very briefly, update generally handles the audio portion of the game and keeps a global variable updated so that render can handle the visual portion of the game.

Update() checks the current song time, defaulting to what it has kept track of as time has elapsed. If an MP3 file is playing, then the tracked song time is trumped by querying the MP3 player for its current time. (This keeps the MIDI file in sync with the ‘real’ audio, should there be buffering delays in the ‘real’ audio playback, for instance.) All MIDI events with timestamps between the last UPDATE and this one are ‘released’ to the MIDI synthesizer (notes on/off, control codes, etc.). Each event that is ‘released’ to the synthesizer also updates gViz.
(see UPDATE_overview.txt, NEW_VISUALIZER_note.cpp, and gVIZ_class.txt)

Render() gathers the ‘orb’ information from gViz and identifies new & recent melody line animations, chord lines, and note animations. Once gathered, that information is rendered to the screen, with ‘recent’ elements aging over time so that they fade away or progress through an animation sequence.
(see RENDER_overview.txt, NEW_VISUALIZER_gather_lines.cpp, gVIZ_class.txt, and m_ActiveLine_structure.txt)




User interaction can short-circuit the normal apparent frame rate. A song can be slowed down or sped up, altering the playback rate of both the MP3 file and the MIDI events. The user can ‘scrub’ forward and backward in a song. While dragging the scrub bar, the MIDI events are visualized, but no MIDI audio is heard. When the scrub bar is released, the playback point is set to the new point. A user can rewind a song back to the beginning (playback is < 0). A user can pause (current time / event is noted, elapsed time is suspended while paused).

Game play can alter the visual / audio aspects. Missing a note in play or practice mode sets the audio of the MP3 file, if there is one, to zero. It fades back in over two seconds. Dependent notes (generally fast sequences of notes or chords) are only heard and seen if the user successfully ‘hit’ the lead note.
