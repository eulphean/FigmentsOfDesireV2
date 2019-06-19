#include "Midi.h"

void Midi::setup() {
  // MIDI setup.
  midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
  channelLeftBack = 2;
  channelLeftFront = 3;
  channelRightBack = 4;
  channelRightFront = 5;
  channelRain = 6;
  channelRightBackMix = 7;
  channelLeftFrontMix = 8;
}

void Midi::sendMidiControlChangeRotary(int device, float val) {
  //if (currentScene == 2 || currentScene == 3) { // Or any scene during which I want to use the dishes, put it here.
    // Map rotary values to Midi signals.
    float midiVal = ofMap(val, 0, 1, 0, 127, true);

    switch (device) {
      case 0: {
        // Channel, control, midi value
        cout << midiVal << endl;
        midiOut.sendControlChange(channelLeftBack, 10, midiVal);
        break;
      }
      
      case 1: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelLeftFront, 11, midiVal);
        break;
      }
      
      case 2: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelRightBack, 12, midiVal);
        break;
      }
      
      case 3: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelRightFront, 13, midiVal);
        break;
      }
      
      case 4: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelRain, 14, midiVal);
        break;
      }
      
      case 5: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelRightBackMix, 15, midiVal);
        break;
      }
      
      case 6: {
        // Channel, control, midi value
        midiOut.sendControlChange(channelLeftFrontMix, 16, midiVal);
        break;
      }
      default:
        break;
    }
  //}
}

// ROtary controls (Midi)
void Midi::sendBondMakeMidi(int midiNote) {
  // Constant velocity
  midiOut.sendNoteOn(bondMakeChannel, midiNote, 64);
}

void Midi::sendBondBreakMidi(int midiNote) {
  // Constant velocity
  midiOut.sendNoteOn(bondBreakChannel, midiNote, 64);
  //midiOut.sendNoteOff(bondMakeChannel, midiNote, 64);
}


void Midi::exit() {
  midiOut.closePort();
}

Midi &Midi::instance() {
  return m;
}

// For a static class, variable needs to be
// initialized in the implementation file.
Midi Midi::m;
