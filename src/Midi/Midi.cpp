#include "Midi.h"

void Midi::setup() {
  // MIDI setup.
  midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
  entryExitChannel = 1;
  agentStretchChannel = 2;
  bondBreakChannel = 1; 
} 

void Midi::sendEntryExitMidi(bool hasEntered) {
  int midiNote = hasEntered ? 48 : 60;
  midiOut.sendNoteOn(entryExitChannel, midiNote, 64);
}

// Every agent will get a unique midi note.
void Midi::sendAgentStretchMidi(int midiNote) {
  midiOut.sendNoteOn(agentStretchChannel, midiNote);
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
