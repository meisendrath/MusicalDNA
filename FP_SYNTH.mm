#include "FP_SYNTH.h"

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

#include <stdio.h>
#include <pthread.h>



//
#define SYNTH_ENABLE

//#define SYNTH_VERBOSE





// State:
typedef struct VoiceDef {
    UInt8 program;				// 0..127: program index; 128 is visualizer, 129 is percussion (if enabled)
	UInt8 sf_patch;				// the specific voice to use from the soundfont (except percussion mode, s/b 0) (should match program index btw 0..127)
	UInt8 channel;
	UInt8 mixer_input_number;
	AUNode samplerNode;			// 
	AudioUnit samplerUnit;
} VoiceDef;
static std::vector<VoiceDef> gVoicesActive;

static AUGraph gProcessingGraph;

static AudioUnit _ioUnit, _mixerUnit;
static AUNode ioNode, mixerNode;

static int gProgram_for_channel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int gProgram_for_vizualizer = 0;
static AudioUnit gSamplerUnit_for_channel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int gMixerInputNumber_for_channel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static AudioUnit gSamplerUnit_for_vizualizer = 0;
static int gMixerInputNumber_for_vizualizer = 0;
static bool gAudioSystemStarted = false;

//

static void program_table_init() {
	for (int i=0; i<16; i++) gProgram_for_channel[i] = 0;
	gProgram_for_vizualizer = 0;
	for (int i=0; i<16; i++) gSamplerUnit_for_channel[i] = 0;
	gSamplerUnit_for_vizualizer = 0;
	for (int i=0; i<16; i++) gMixerInputNumber_for_channel[i] = 0;
	gMixerInputNumber_for_vizualizer = 0;
	_ioUnit = _mixerUnit = nil;
	ioNode = mixerNode = (AUNode)NULL;
	gVoicesActive.resize(0);
}

// Load a SoundFont into a sampler
static OSStatus loadFromDLSOrSoundFont(NSURL *bankURL, UInt8 bank, int presetNumber, AudioUnit sampler) {
    OSStatus result = noErr;
    
    // fill out a bank preset data structure
    AUSamplerBankPresetData bpdata;
    bpdata.bankURL  = (CFURLRef) bankURL;
    bpdata.bankMSB  = bank;
    bpdata.bankLSB  = kAUSampler_DefaultBankLSB;
    bpdata.presetID = (UInt8) presetNumber;
    
    // set the kAUSamplerProperty_LoadPresetFromBank property
    result = AudioUnitSetProperty(sampler,
                                  kAUSamplerProperty_LoadPresetFromBank,
                                  kAudioUnitScope_Global,
                                  0,
                                  &bpdata,
                                  sizeof(bpdata));
    
    // check for errors
    if (result != noErr) printf("Unable to set the preset property on the Sampler. Error code:%d '%.4s'", (int) result, (const char *)&result);
    
    return result;
}

static void loadSoundFont(NSString *path, int patch, UInt8 bank, AudioUnit sampler) {    
//    NSLog(@"Sound font: %@", path);
//printf("loadSoundFont: %s\n", [path UTF8String]);
	NSString *docPath = [NSSearchPathForDirectoriesInDomains (NSDocumentDirectory,NSUserDomainMask, YES) objectAtIndex:0];
	NSString *filePath = [NSString stringWithFormat:@"%@/%@.sf2", docPath, path];
	BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:filePath];

	NSString *fn;
	if (fileExists) fn = filePath; else
#ifdef USE_MINIMAL_MEMORY
	fn = [[NSBundle mainBundle] pathForResource:path ofType:@"sf2"];
#else // !USE_MINIMAL_MEMORY
	fn = [[NSBundle mainBundle] pathForResource:path ofType:@"sf2" inDirectory:@"sf2"];
#endif // !USE_MINIMAL_MEMORY

	if (fn == nil) return;
	
    NSURL *presetURL = [[NSURL alloc] initFileURLWithPath:fn];
//printf("     URL = %s\n", [[presetURL absoluteString] UTF8String]);
    loadFromDLSOrSoundFont(presetURL, bank, patch, sampler);
    [presetURL relativePath];
    [presetURL release];
}

// Create a new audio graph containing all the added voices
static void startAudioGraph(std::vector<VoiceRequest> &voicereqs) {		// no dups, DO append elements for percussion and/or visualizer
	OSStatus result = noErr;

// *** call NewAUGraph()
    
    // Specify the common portion of an audio unit's identify, used for both audio units
    // in the graph.
	AudioComponentDescription cd = {};
	cd.componentManufacturer     = kAudioUnitManufacturer_Apple;
    
    // Instantiate an audio processing graph
	result = NewAUGraph (&gProcessingGraph);
	if (result != noErr) printf("Unable to create an AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);
    
// *** call AUGraphAddNode() for all requested voices

	//Specify the Sampler unit, to be used as the first node of the graph
	cd.componentType = kAudioUnitType_MusicDevice;
	cd.componentSubType = kAudioUnitSubType_Sampler;
	
	for (int i=0; i<voicereqs.size(); i++) {
		AUNode sampler;
        result = AUGraphAddNode (gProcessingGraph, &cd, &sampler);
        if (result != noErr) printf("Unable to add the Sampler unit to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
		VoiceDef vd;
		vd.program = voicereqs[i].program;
		vd.sf_patch = voicereqs[i].sf_patch;
		vd.channel = voicereqs[i].channel;
		vd.mixer_input_number = 0;
		vd.samplerNode = sampler;
		vd.samplerUnit = nil;
		gVoicesActive.push_back(vd);
	}

// *** call AUGraphAddNode() for the output and mixer nodes

	// Specify the Output unit, to be used as the second and final node of the graph	
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_RemoteIO;  

    // Add the Output unit node to the graph
	result = AUGraphAddNode (gProcessingGraph, &cd, &ioNode);
    if (result != noErr) printf("Unable to add the Output unit to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
    
    // Add the mixer unit to the graph
    cd.componentType = kAudioUnitType_Mixer;
    cd.componentSubType = kAudioUnitSubType_MultiChannelMixer;
    
    result = AUGraphAddNode (gProcessingGraph, &cd, &mixerNode);
    if (result != noErr) printf("Unable to add the Output unit to the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);

// *** call AUGraphOpen() to open the graph

    // Open the graph
	result = AUGraphOpen (gProcessingGraph);
    if (result != noErr) printf("Unable to open the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);

// *** call AUGraphNodeInfo() to create the sampler unit and then load the sound font for each requested voice

//	NSString *sf = @"vintage";
//	NSString *sf = @"chorium";
//	NSString *sf = @"fluid_gm";
#ifdef USE_MINIMAL_MEMORY
	NSString *sf = @"synergi";
#else // !USE_MINIMAL_MEMORY
	NSString *sf = @"soundfont";
#endif // !USE_MINIMAL_MEMORY
	for (int i=0; i<gVoicesActive.size(); i++) {
		AudioUnit samplerUnit;
        // Get a reference to the sampler node and store it in the samplerUnit variable
        result = AUGraphNodeInfo (gProcessingGraph, gVoicesActive[i].samplerNode, 0, &samplerUnit);
        if (result != noErr) printf("Unable to obtain a reference to the Sampler unit. Error code: %d '%.4s'", (int) result, (const char *)&result);

        // Load the soundfont for our sampler unit using the voice definition
		switch (gVoicesActive[i].program) {
			default:		loadSoundFont(sf, gVoicesActive[i].sf_patch, kAUSampler_DefaultMelodicBankMSB, samplerUnit);		break;
			case 129:		loadSoundFont(sf, 0, kAUSampler_DefaultPercussionBankMSB, samplerUnit);								break;
		}
#ifdef SYNTH_VERBOSE
printf("%2d: pgm%d patch=%d ch=%d\n", i, gVoicesActive[i].program, gVoicesActive[i].sf_patch, gVoicesActive[i].channel);
#endif // SYNTH_VERBOSE

		gVoicesActive[i].samplerUnit = samplerUnit;
		int ch = gVoicesActive[i].channel;
		if (ch == 128) {
			gSamplerUnit_for_vizualizer = gVoicesActive[i].samplerUnit;
		} else if (ch == 129) {
//			gSamplerUnit_for_ = gVoicesActive[i].samplerUnit;		// percussion?
		} else if (ch < 16) {
			gSamplerUnit_for_channel[gVoicesActive[i].channel] = gVoicesActive[i].samplerUnit;
#warning seems wrong because why should viz be tied to ch0?...
			if (gVoicesActive[i].channel == 0) gSamplerUnit_for_vizualizer = gVoicesActive[i].samplerUnit;
		}
	}

// *** call AUGraphNodeInfo() to create the mixer and output units
    
    // Create a new mixer unit. This is necessary because we have a number of sampler
    // units which we need to output through the speakers. Each of these channels needs
    // to be mixed together to create one output
	result = AUGraphNodeInfo (gProcessingGraph, mixerNode, 0, &_mixerUnit);
    if (result != noErr) printf("Unable to obtain a reference to the Sampler unit. Error code: %d '%.4s'", (int) result, (const char *)&result);

	// Obtain a reference to the I/O unit from its node
	result = AUGraphNodeInfo (gProcessingGraph, ioNode, 0, &_ioUnit);
    if (result != noErr) printf("Unable to obtain a reference to the I/O unit. Error code: %d '%.4s'", (int) result, (const char *)&result);
    
// *** call AudioUnitSetProperty() to set the number of inputs in the mixer unit
    
    // Define the number of input busses
    UInt32 busCount   = (UInt32)gVoicesActive.size();
    
    // Set the input channels property on the mixer unit
    result = AudioUnitSetProperty (_mixerUnit,
                                   kAudioUnitProperty_ElementCount,
                                   kAudioUnitScope_Input,
                                   0,
                                   &busCount,
                                   sizeof (busCount)
                                   );
    if (result != noErr) printf("AudioUnitSetProperty Set mixer bus count. Error code: %d '%.4s'", (int) result, (const char *)&result);

// *** call AUGraphConnectNodeInput() to connect the sampler/voice nodes to the mixer

    // Connect the nodes to the mixer node
	int j = 0;
	for (int i=0; i<gVoicesActive.size(); i++) {
		gVoicesActive[i].mixer_input_number = (UInt8)j;
		int ch = gVoicesActive[i].channel;
		if (ch == 128) {
			gMixerInputNumber_for_vizualizer = gVoicesActive[i].mixer_input_number;
		} else if (ch == 129) {
//			gMixerInputNumber_for_ = gVoicesActive[i].mixer_input_number;		// percussion?
		} else if (ch < 16) {
			gMixerInputNumber_for_channel[gVoicesActive[i].channel] = gVoicesActive[i].mixer_input_number;
#warning seems wrong because why should viz be tied to ch0?...
			if (gVoicesActive[i].channel == 0) gMixerInputNumber_for_vizualizer = gVoicesActive[i].mixer_input_number;
		}

        // Connect the sampler unit to the mixer unit
        result = AUGraphConnectNodeInput(gProcessingGraph, gVoicesActive[i].samplerNode, 0, mixerNode, j);
        if (result != noErr) printf("Couldn't connect audio unit output (0) to mixer input (%d) for program %d. Error code: %d '%.4s'", j, i, (int) result, (const char *)&result);

        // Set the volume of the channel
        AudioUnitSetParameter(_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, j, 1.0f, 0);        
        if (result != noErr) printf("Couldn't set mixer input (%d) volume to 1.0f for program %d. Error code: %d '%.4s'", j, i, (int) result, (const char *)&result);

		j++;
	}

// *** call AUGraphConnectNodeInput() to connect the mixer to the output node
    
    // Connect the output of the mixer node to the input of the io node
    result = AUGraphConnectNodeInput (gProcessingGraph, mixerNode, 0, ioNode, 0);
    if (result != noErr) printf("Unable to interconnect the nodes in the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);

// *** done
    
    // Print a graphic version of the graph
#ifdef SYNTH_VERBOSE
    CAShow(gProcessingGraph);
#endif // SYNTH_VERBOSE

    if (gProcessingGraph) {

// *** call AUGraphInitialize()

        // Initialize the audio gProcessingGraph graph.
        result = AUGraphInitialize (gProcessingGraph);
        if (result != noErr) printf("Unable to initialze AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);
        
// *** call AUGraphStart()

        // Start the graph
        result = AUGraphStart (gProcessingGraph);
        if (result != noErr) printf("Unable to start audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);
    }
}


static void stopAudioGraph() {
	OSStatus result = AUGraphStop (gProcessingGraph);
	if (result != noErr) printf("Unable to stop audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);

	result = AUGraphUninitialize (gProcessingGraph);
	if (result != noErr) printf("Unable to uninitialze AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);

	result = AUGraphClose (gProcessingGraph);
	if (result != noErr) printf("Unable to close the audio processing graph. Error code: %d '%.4s'", (int) result, (const char *)&result);

	result = DisposeAUGraph (gProcessingGraph);
	if (result != noErr) printf("Unable to dispose the AUGraph object. Error code: %d '%.4s'", (int) result, (const char *)&result);
}



//



////////////////////////////////////




static void FP_SYNTH_on_demand_setup() {
#ifdef SYNTH_ENABLE
	if (!gAudioSystemStarted) {		// default set up, no MIDI has played yet...
		std::vector<VoiceRequest> voicereqs;
		VoiceRequest vr;
		vr.program = 0;			vr.sf_patch = 0;		vr.channel = 0;		voicereqs.push_back(vr);
		vr.program = 128;		vr.sf_patch = 0;		vr.channel = 128;	voicereqs.push_back(vr);	// visualizer
		vr.program = 129;		vr.sf_patch = 0;		vr.channel = 129;	voicereqs.push_back(vr);	// channel 9 is percussion
		FP_SYNTH_setup_with_voices(voicereqs);
	}
#endif // SYNTH_ENABLE
}

void FP_SYNTH_bootup_init() {
#ifdef SYNTH_ENABLE
	program_table_init();
	FP_SYNTH_on_demand_setup();
#endif // SYNTH_ENABLE
}

extern double getcurrenttime();
bool g_voice_setup_mutex_initted = false;
pthread_mutex_t g_voice_setup_mutex;

void FP_SYNTH_setup_with_voices(std::vector<VoiceRequest> &voicereqs) {
	if (!g_voice_setup_mutex_initted) {
		pthread_mutex_init(&g_voice_setup_mutex, NULL);
		g_voice_setup_mutex_initted = true;
	}

	while (pthread_mutex_trylock(&g_voice_setup_mutex) != 0) {
		pthread_yield_np();
	}

#ifdef SYNTH_ENABLE
double timeat = getcurrenttime();
printf(" @@@ startAudioGraph starts\n");
	if (gAudioSystemStarted) {
        stopAudioGraph();

		program_table_init();
		
		gAudioSystemStarted = false;
	}

	startAudioGraph(voicereqs);
printf(" @@@ startAudioGraph took: %lg seconds\n", getcurrenttime()-timeat); timeat = getcurrenttime();

	gAudioSystemStarted = true;
#endif // SYNTH_ENABLE

	pthread_mutex_unlock(&g_voice_setup_mutex);
}

void FP_SYNTH_all_notes_off() {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_all_notes_off()\n");
#endif // SYNTH_VERBOSE
	if (!gAudioSystemStarted) return;
//	[[MIDISynthesizer sharedSynthesizer] allNotesOff];
	for (int n=0; n<128; n++) {
		for (int j=0; j<gVoicesActive.size(); j++) {
			MusicDeviceMIDIEvent(gVoicesActive[j].samplerUnit, 0x080, n, 0, 0);
		}
	}
#endif // SYNTH_ENABLE
}

void FP_SYNTH_set_patch(int ch, int inst, bool is_from_visualizer) {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_set_patch(%d, %d, %s)\n", ch, inst, is_from_visualizer?"FROM_VIZ":"");
#endif // SYNTH_VERBOSE
	int j = -1;
	if (is_from_visualizer) {
		gProgram_for_vizualizer = inst;
		for (j=0; j<gVoicesActive.size(); j++) if ((128 == gVoicesActive[j].channel) && (inst == gVoicesActive[j].program)) break;
		if (j < gVoicesActive.size()) {
			gSamplerUnit_for_vizualizer = gVoicesActive[j].samplerUnit;
			gMixerInputNumber_for_vizualizer = gVoicesActive[j].mixer_input_number;
		}
	} else {
		gProgram_for_channel[ch] = inst;
		for (j=0; j<gVoicesActive.size(); j++) if ((ch == gVoicesActive[j].channel) && (inst == gVoicesActive[j].program)) break;
		if (j < gVoicesActive.size()) {
			gSamplerUnit_for_channel[ch] = gVoicesActive[j].samplerUnit;
			gMixerInputNumber_for_channel[ch] = gVoicesActive[j].mixer_input_number;
		}
	}
#endif // SYNTH_ENABLE
}

int FP_SYNTH_get_patch(int ch, bool is_from_visualizer) {
#ifdef SYNTH_ENABLE
	if (is_from_visualizer) return gProgram_for_vizualizer;
	return gProgram_for_channel[ch];
#else // SYNTH_ENABLE
	return 0;
#endif // SYNTH_ENABLE
}

void FP_SYNTH_control(int ch, int ctrl, int data, bool is_from_visualizer) {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_control(%d, %d, %s)\n", ch, ctrl, is_from_visualizer?"FROM_VIZ":"");
#endif // SYNTH_VERBOSE
	if (!gAudioSystemStarted) return;
	switch (ctrl) {
		case 7:
		case 19:
// NOTE: do mixing by computing a real volume for FP_SYNTH_rawplay()
//			if (is_from_visualizer) {
//				mi = gMixerInputNumber_for_vizualizer;
//			} else {
//				mi = gMixerInputNumber_for_channel[ch];
//			}
//			AudioUnitSetParameter(_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, mi, data/127.0f, 0);
			break;
	}
#endif // SYNTH_ENABLE
}

void FP_SYNTH_pitchbend(int ch, int data, bool is_from_visualizer) {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_pitchbend(%d, %d)\n", ch, data);
#endif // SYNTH_VERBOSE
	FP_SYNTH_on_demand_setup();
	AudioUnit sampler;
	if (is_from_visualizer) {
		sampler = gSamplerUnit_for_vizualizer;
	} else {
		sampler = gSamplerUnit_for_channel[ch];
	}
	MusicDeviceMIDIEvent(sampler, 0xe0, 0x7f & (data >> 7), 0x7f & data, 0);
#endif // SYNTH_ENABLE
}

void FP_SYNTH_set_global_volume(float vol) {
#ifdef SYNTH_ENABLE
	AudioUnitSetParameter(_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, vol, 0);        
#endif // SYNTH_ENABLE
}

void FP_SYNTH_rawplay(int ch, int notenum, int vol, bool is_from_visualizer) {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_rawplay(%d, %d, %d, %s)\n", ch, notenum, vol, is_from_visualizer?"FROM_VIZ":"");
#endif // SYNTH_VERBOSE
	FP_SYNTH_on_demand_setup();
	AudioUnit sampler;
	if (is_from_visualizer) {
		sampler = gSamplerUnit_for_vizualizer;
	} else {
		sampler = gSamplerUnit_for_channel[ch];
	}
	MusicDeviceMIDIEvent(sampler, vol==0?0x080:0x90, notenum, vol, 0);
////	[[MIDISynthesizer sharedSynthesizer] play:notenum:vol:((ch<15)?ch:15)];
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_rawplay(%d, %d, %d, %s) done\n", ch, notenum, vol, is_from_visualizer?"FROM_VIZ":"");
#endif // SYNTH_VERBOSE
#endif // SYNTH_ENABLE
}

void FP_SYNTH_rawplay_drums(int ch, int notenum, int vol) {
#ifdef SYNTH_ENABLE
#ifdef SYNTH_VERBOSE
printf("FP_SYNTH_rawplay_drums(%d, %d, %d)\n", ch, notenum, vol);
#endif // SYNTH_VERBOSE
	FP_SYNTH_on_demand_setup();
	AudioUnit sampler = nil;
	for (int j=0; j<gVoicesActive.size(); j++) if (129 == gVoicesActive[j].program) sampler = gVoicesActive[j].samplerUnit;
	if (sampler != nil) MusicDeviceMIDIEvent(sampler, vol==0?0x080:0x90, notenum, vol, 0);
#endif // SYNTH_ENABLE
}
