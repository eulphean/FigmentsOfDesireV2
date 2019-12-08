
#include "Instrument.h"

void Instrument::patch() {
    // Add inputs / outputs with these methods
    addModuleInput("trig", env.in_trig()); // arguments are tag and the Unit in/out to link to that tag
    addModuleInput("pitch", osc.in_pitch());
    addModuleInput("freq", lfo.in_freq());
    addModuleInput("cutoff", filter.in_cutoff());
    addModuleOutput("signal", amp); // if in/out is not selected default in/out is used
  
  
    // ------------ Patching -------------
    
    // a pdsp::ADSR is an ADSR envelope that makes a one-shot modulation when triggered
    // pdsp::ADSR require an output sending trigger signals
    // remember, in pdsp out_trig() always have to be connected to in_trig()
    // in_trig() is the default pdsp::ADSR input signal
    // ADSR
    env.set(0.f, 250.f, dB(0.0), 5.0 * 1000.f);
    env.setAttackCurve(0.0f);
    env.setReleaseCurve(0.5f);
    env.setCurve(0.5f);
    env >> amp.in_mod(); // Enable / Disable the sound based on trigger.
  
    // LFO & Filter.
    lfo.out_sample_and_hold() >> filter.in_reso();
  
    // Oscillator
    osc.out_sine() >> filter >> amp;
}

pdsp::Patchable& Instrument::in_trig(){
    return in("trig");
}

pdsp::Patchable& Instrument::in_pitch(){
    return in("pitch");
}

pdsp::Patchable& Instrument::out_signal(){
    return out("signal");
}

pdsp::Patchable& Instrument::in_freq() {
    return in("freq");
}

pdsp::Patchable& Instrument::in_cutoff() {
    return in("cutoff");
}

float const Instrument::meterOut() {
  return env.meter_output();
}
