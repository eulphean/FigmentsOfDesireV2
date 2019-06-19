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
    void sendBondMakeMidi(int midiNote);
    void sendBondBreakMidi(int midiNote);
    void sendMidiControlChangeRotary(int device, float val);
    
    static Midi &instance();
    
  private:
    ofxMidiOut midiOut;
    static Midi m;
    int bondMakeChannel, bondBreakChannel;
    int channelLeftBack;
    int channelLeftFront;
    int channelRightBack;
    int channelRightFront;
    int channelRain;
    int channelRightBackMix;
    int channelLeftFrontMix;
};
