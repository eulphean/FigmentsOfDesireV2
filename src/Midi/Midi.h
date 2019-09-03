// Singleton pattern for handling Midi calls by
// multiple components.

#pragma once
#include "ofMain.h"
#include "ofxMidi.h"

class Midi {
  public:
    void setup();
    void exit();
  
    // Midi note assignment for Stretch
    int assignMidiNote();
  
    // Midi calls. 
    void sendAgentExplosionMidi();
    void sendAgentStretchMidi(int midiNote, bool isOn);
    
    static Midi &instance();
    
  private:
    ofxMidiOut midiOut;
    static Midi m;
    int agentExplodeChannel;
    int agentStretchChannel;
    
    std::vector<int> stretchMidiNotes;
    std::vector<int> assignedMidiNotes;
};
