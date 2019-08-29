#include "Midi.h"

void Midi::setup() {
  // MIDI setup.
  midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
  entryExitChannel = 2;
  agentStretchChannel = 1;
  bondBreakChannel = 1;
  
  // Midi notes
  stretchMidiNotes = { 36, 39, 40, 41, 43, 47,
                       48, 51, 52, 53, 55, 58,
                       60, 53, 64, 65, 67, 70 };
  assignedMidiNotes = { }; 
} 

void Midi::sendEntryExitMidi(bool hasEntered) {
  int midiNote = hasEntered ? 48 : 60;
  midiOut.sendNoteOn(entryExitChannel, midiNote, 30);
}

// Every agent will get a unique midi note.
void Midi::sendAgentStretchMidi(int midiNote, bool isOn) {
  if (isOn) {
    midiOut.sendNoteOn(agentStretchChannel, midiNote);
  } else {
    midiOut.sendNoteOff(agentStretchChannel, midiNote);
  }
}

int Midi::assignMidiNote() {
  int randIdx = ofRandom(stretchMidiNotes.size());
  int note = stretchMidiNotes.at(randIdx);
  while (ofContains(assignedMidiNotes, note)) { // Does it contain that value?
    randIdx = ofRandom(stretchMidiNotes.size());
    note = stretchMidiNotes.at(randIdx);
  }
  return note;
}

void Midi::exit() {
  midiOut.closePort();
}

Midi &Midi::instance() {
  return m;
}

Midi Midi::m;
