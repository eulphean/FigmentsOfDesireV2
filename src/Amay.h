// Amay is a synthetic agent carrying information regarding Amay's behaviors and traits. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Amay : public Agent {
  public:
    Amay(ofxBox2d &box2d, AgentProperties agentProps);
};

