// Azra is a synthetic agent carrying information regarding Azra's behavior and traits. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Azra : public Agent {
  public:
    Azra(ofxBox2d &box2d, AgentProperties agentProps);
};
