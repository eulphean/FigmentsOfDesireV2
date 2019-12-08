
#pragma once

#include "ofxPDSP.h"

// to create your own modules, you have to extend pdsp::Patchable

class Instrument : public pdsp::Patchable{
  
public:
    // Defualt constructor.
    Instrument() { patch(); } // default constructor
    // Copy constructor
    Instrument( const Instrument & other ) { patch(); } // you need this to use std::vector with your class,
  
    void patch ();
  
    // Inputs
    pdsp::Patchable & in_pitch();
    pdsp::Patchable & in_trig();
    pdsp::Patchable & in_freq();
    pdsp::Patchable & in_cutoff();
  
    // Outputs
    pdsp::Patchable& out_signal();
    float const meterOut();
  
private:
    // Oscillators
    pdsp::VAOscillator      osc;
  
    pdsp::Amp               amp;
    pdsp::VAFilter          filter;
    pdsp::ADSR              env;
    pdsp::LFO               lfo;
};
