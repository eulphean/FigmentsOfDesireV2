
#include "Instrument.h"

Instrument::Instrument () {
  // Select a pitch that will drive this instrument
  patch();
  
  // Sine = 0
  // Saw = 1
  // Triangle = 2
  // Pulse = 3
}

void Instrument::patch() {
    // Add inputs / outputs with these methods
    addModuleInput("pw", osc.in_pw());
    addModuleInput("trig", env.in_trig()); // arguments are tag and the Unit in/out to link to that tag
    addModuleInput("attack", env.in_attack());
    addModuleInput("decay", env.in_decay());
    addModuleInput("sustain", env.in_sustain());
    addModuleInput("release", env.in_release());
    addModuleInput("velocity", env.in_velocity());
    addModuleOutput("signal", amp); // if in/out is not selected default in/out is used
  
    // Select a pitch
    float startingPitch = pdsp::f2p(150.f);
    float endingPitch = pdsp::f2p(700.f);
    float pitch = ofRandom(startingPitch, endingPitch);
    pitch_ctrl.set(pitch);
  
    // ------------ Patching -------------

    // ADSR
    env.setAttackCurve(0.0f);
    env.setReleaseCurve(0.5f);
    env.setCurve(0.5f);
    env >> amp.in_mod(); // Enable / Disable the sound based on trigger.
  
    // Oscillator
    pitch_ctrl >> osc.in_pitch();
  
    if (ofRandom(1) < 0.35) {
      osc.out_sine() >> amp;
    } else {
      osc.out_triangle() >> amp;
    }
}

pdsp::Patchable& Instrument::in_trig(){
    return in("trig");
}

pdsp::Patchable& Instrument::in_attack(){
    return in("attack");
}

pdsp::Patchable& Instrument::in_decay(){
    return in("decay");
}

pdsp::Patchable& Instrument::in_release(){
    return in("release");
}

pdsp::Patchable& Instrument::in_sustain(){
    return in("sustain");
}

pdsp::Patchable& Instrument::in_velocity(){
    return in("velocity");
}

pdsp::Patchable& Instrument::out_signal(){
    return out("signal");
}

pdsp::Patchable& Instrument::in_pw(){
    return in("pw");
}
