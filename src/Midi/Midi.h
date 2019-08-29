// Singleton pattern for handling Midi calls by
// multiple components.

#pragma once
#include "ofMain.h"
#include "ofxMidi.h"

class Midi {
  public:
    void setup();
    void exit();
  
    // Midi calls. 
    void sendEntryExitMidi(bool isEntering);
    void sendAgentStretchMidi(int midiNote);
    
    static Midi &instance();
    
  private:
    ofxMidiOut midiOut;
    static Midi m;
    int entryExitChannel;
    int agentStretchChannel;
    int bondBreakChannel;
};
