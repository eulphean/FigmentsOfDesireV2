
#pragma once

#include "ofxPDSP.h"

class Instrument : public pdsp::Patchable{
  
public:
    // Default constructor.
    Instrument();
    // Copy constructor
    Instrument( const Instrument & other ) { patch(); } // you need this to use std::vector with your class,
  
    void patch ();
  
    // Inputs
    pdsp::Patchable & in_trig();
    pdsp::Patchable & in_pw();
    pdsp::Patchable & in_attack();
    pdsp::Patchable & in_decay();
    pdsp::Patchable & in_release();
    pdsp::Patchable & in_sustain();
    pdsp::Patchable & in_velocity();

    // Outputs
    pdsp::Patchable& out_signal();
  
private:
    // Oscillators
    pdsp::VAOscillator      osc;
    pdsp::Amp               amp;
    pdsp::ADSR              env;
  
    // Pitch at which this instrument is played
    pdsp::ValueControl pitch_ctrl;
};
